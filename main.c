#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <conio.h>
/*
    Apenas um typedef para melhorar o uso de valores booleanos.
    FALSE == 0, TRUE == 1
 */
typedef enum {
	FALSE, TRUE
} bool;


/*
   pair <int key, void* value>

   Como o valor associado à chave é um ponteiro void,
   a B-Tree terá capacidade de armazenar qualquer tipo de dado
   indexável por uma chave inteira.
 */
typedef struct pair_t {
	int key;
	void *value;
} pair_t;

/*
   Nó da B-Tree
   is_leaf:    esse é um nó-folha?
   n_keys:     número de chaves ativas

   children:   ponteiro para a lista de (ponteiros para) nós-filhos
   keys:       ponteiro para a lista de (ponteiros para) pairs<key, value>
 */
typedef struct node_t {
	bool is_leaf;
	int n_keys;

	struct node_t **children;
	pair_t **keys;
} node_t;

/*
   Posição de uma key/elemento da B-Tree
   Atenção ao uso:
    Expõe acesso direto à um nó da B-Tree, incluindo suas chaves e nós-filhos

   node:    ponteiro para o nó
   index:   indice da key dentro do nó
 */
typedef struct node_position_t {
	node_t *node;
	int index;
} node_position;

/*
    Cria um novo node_position com os valores passados
 */

node_position _node_position_new(node_t *node, int index) {
	// Apenas retorna um node_position com os valores informados
	node_position node_pos = {.node = node, .index = index};
	return node_pos;
}

/*
    Aloca e retorna um novo pair com os valores passados.
 */

pair_t* _pair_new(int key, void *value) {
	pair_t *p = malloc(sizeof(pair_t));
	assert(p != NULL);

	p->key = key;
	p->value = value;

	return p;
}
/*
    Aloca e retorna um novo pair com os valores do pair passado como parâmetro
 */
pair_t* _pair_copy(pair_t *p) {
	// Apenas crie um novo pair_t com os mesmos valores que o antigo
	pair_t *new = _pair_new(p->key, p->value);
	return new;
}
/*
    Aloca e retorna um novo node, com espaço para até 2*order-1 ponteiros .
    para pair's e 2*order ponteiros para node's.
    O parâmetro is_leaf é atribuído ao atributo de mesmo nome no node.
 */
node_t* _node_new(int order, bool is_leaf) {
	node_t *n = malloc(sizeof(node_t));
	assert(n != NULL);

	n->n_keys = 0;
	n->is_leaf = is_leaf;
	n->keys = malloc((2*order-1) * sizeof(pair_t));
	n->children = malloc((2*order) * sizeof(node_t*));

	return n;
}
/*
    node:   nó em que a key será pesquisada
    key:    chave à se procurar
    pos:    retorno da posição em que a chave foi encontrada. Default: -1
    @return: se a chave key pertence ou não ao nó node
 */
bool _node_find_key(node_t *node, int key, int *pos) {
	*pos = 0;

	// Encontra a posição cuja chave é imediatamente maior ou igual à key
	// (respeitando o número de chaves ativas no nó)
	while ((*pos) < node->n_keys && key > node->keys[(*pos)]->key) {
		(*pos)++;
	}

	// A chave foi encontrada apenas se a posição (*pos)
	// pertence ao nó e se a chave está nessa posição
	return (*pos) < node->n_keys && key == node->keys[(*pos)]->key;
}
/*
    As funções a seguir correspondem à execução de um FOR.
    Tratam-se apenas de funções auxiliares para aumentar a legibilidade
    do código principal (remove_key)
 */
void _node_deslocate_keys_up(node_t *to, node_t *from, int beg, int end, int padding_to, int padding_from) {

	int j;
	for (j = beg; j < end; j++) {
		to->keys[j + padding_to] = from->keys[j + padding_from];
	}
}

void _node_deslocate_keys_down(node_t *to, node_t *from, int beg, int end, int padding_to, int padding_from) {

	int j;
	for (j = beg; j > end; j--) {
		to->keys[j + padding_to] = from->keys[j + padding_from];
	}

}

void _node_deslocate_children_up(node_t *to, node_t *from, int beg, int end, int padding_to, int padding_from) {
	int j;

	for (j = beg; j < end; j++) {
		to->children[j + padding_to] = from->children[j + padding_from];
	}

}

void _node_deslocate_children_down(node_t *to, node_t *from, int beg, int end, int padding_to, int padding_from) {

	int j;
	for (j = beg; j > end; j--) {
		to->children[j + padding_to] = from->children[j + padding_from];

	}

}

node_position _node_find_max(node_t *node) {
	assert(node != NULL);

	if (node->is_leaf) {
		return _node_position_new(node, node->n_keys-1);
	}
	else {
		return _node_find_max(node->children[node->n_keys]);
	}
}

node_position _node_find_min(node_t *node) {
	assert(node != NULL);

	if (node->is_leaf) {
		return _node_position_new(node, 0);
	}
	else {
		return _node_find_max(node->children[0]);
	}
}
/*
    Deleta um node alocado pela função _node_new(),
    isto é, essa função não tem como responsabilidade deletar
    a chaves e os nós-filhos associados à node.
 */
void _node_delete(node_t *node) {
	free(node->children);
	free(node->keys);
	free(node);
}



/*
   B-Tree
   order:  número da ordem da B-Tree (cada nó da B-Tree possui [order-1, 2*order] filhos)
   root:   ponteiro para o nó raiz da B-Tree (possui [2, 2*order] filhos)
 */
typedef struct btree_t {
	int order;
	node_t *root;
} BTree;

/*
    Inicializa uma B-Tree de ordem order.
    Utilizada como sub-procedimento na função btree_new().
    Pode ser utilizada para inicializar uma B-Tree criada na Stack.
 */
void btree_init(BTree *bt, int order) {
	assert(bt != NULL);

	bt->order = order;
	bt->root = _node_new(order, TRUE);
}

/*
    Aloca e retorna uma B-Tree na Heap de ordem order
 */

BTree* btree_new(int order) {
	BTree* bt = malloc(sizeof(BTree));
	assert(bt != NULL);
	// Após alocar, temos que inicializar a B-Tree
	btree_init(bt, order);

	return bt;
}
/*
    Procura a key na sub-árvore com raiz node.
    Retorna um node_position para permitir a recuperação do valor
    associado à key.
    Retorna node_position(NULL, -1) caso key não seja encontrada.
 */
node_position _btree_find_node(node_t* node, int key) {
	assert(node != NULL);

	int pos;
	if (_node_find_key(node, key, &pos)) {
		// Se a chave foi encontrada nesse nó,
		// retorne um meio de acessá-la
		return _node_position_new(node, pos);
	}
	else {
		// Se a chave não foi encontrada
		if (node->is_leaf) {
			// e o nó atual é uma folha,
			// então key não pertence à B-Tree
			return _node_position_new(NULL, -1);
		}
		else {
			// e o nó atual possui filhos,
			// então devemos explor o filho na posição pos
			return _btree_find_node(node->children[pos], key);
		}
	}
}

/*
    Procura a chave key na B-Tree bt.
    Chama a função _btree_find_node() sobre a raiz de bt.
 */
node_position btree_find(BTree* bt, int key) {
	assert(bt != NULL);

	return _btree_find_node(bt->root, key);
}

/*
    Realiza a função split sobre x, na posição pos.
    order:  ordem da árvore à qual node pertence. Utilizado para alocar um novo nó.
 */
void _btree_split(node_t *node, int pos, int order) {

	node_t *y = node->children[pos];
	assert(y != NULL);

	node_t *z = _node_new(order, y->is_leaf);
	z->n_keys = order-1;

	_node_deslocate_keys_up(z, y, 0, order-1, 0, order);

	if (!y->is_leaf) {
		_node_deslocate_children_up(z, y, 0, order, 0, order);
	}
	y->n_keys = order-1;

	_node_deslocate_children_down(node, node, node->n_keys, pos, 1, 0);

	node->children[pos+1] = z;

	_node_deslocate_keys_down(node, node, node->n_keys-1, pos-1, 1, 0);

	node->keys[pos] = y->keys[order-1];

	node->n_keys++;
}

/*
    Trata todos os casos de inserção de um pair na B-Tree.
    Retorna um node_position indicando onde a chave foi inserida.
    Retorna node_position(NULL, -1) caso key já exista em bt.

    order:  ordem da árvore à qual node pertence. Utilizado para alocar um novo nó.
 */
node_position _btree_insert_nonfull(node_t * node, pair_t *pair, int order) {

	int pos = node->n_keys - 1;
	if (node->is_leaf) {

		while (pos >= 0 && pair->key < node->keys[pos]->key) {
			node->keys[pos+1] = node->keys[pos];
			pos--;
		}

		if (pos >= 0 && pair->key == node->keys[pos]->key) {

			pos++;
			while (pos != node->n_keys) {
				node->keys[pos] = node->keys[pos+1];
				pos++;
			}
			return _node_position_new(NULL, -1);
		}
		else {
			pos++;

			node->keys[pos] = pair;
			node->n_keys++;
			return _node_position_new(node, pos);
		}
	}
	else {
		while (pos >= 0 && pair->key < node->keys[pos]->key) {
			pos--;
		}
		if (pos >= 0 && pair->key == node->keys[pos]->key) {

			return _node_position_new(NULL, -1);
		}
		else {
			pos++;

			if (node->children[pos]->n_keys == 2*order-1) {
				_btree_split(node, pos, order);
				if (pair->key > node->keys[pos]->key) {
					pos++;
				}
			}

			return _btree_insert_nonfull(node->children[pos], pair, order);
		}
	}
}

/*
    Insere a chave key na B-Tree bt, associando-a ao valor value.
    Trata o caso em que a raiz está cheia analisando a raiz
    e chamando (possivelmente) a função _btree_split().
    Chama a função _btree_insert_nonfull() sobre a raiz de bt.
 */
node_position btree_insert(BTree* bt, int key, void *value) {
	assert(bt != NULL);
	node_t *root = bt->root;

	// Esse pair será enviado durante as chamadas recursivas de inserção,
	// e é o que realmente será inserido na B-Tree
	pair_t *pair = _pair_new(key, value);

	if (root->n_keys == 2*bt->order -1) {
		// Caso a raiz da B-Tree já esteja cheia,
		// devemos executar o procedimento de split
		// e deixá-la com apenas uma chave.
		// Esse é o único caso em que a altura da B-Tree aumenta
		node_t *new_root = _node_new(bt->order, FALSE);
		new_root->children[0] = root;

		_btree_split(new_root, 0, bt->order);
		bt->root = new_root;

		// Podemos prosseguir com a inserção
		return _btree_insert_nonfull(new_root, pair, bt->order);
	}
	else {
		// A raiz respeita a restrição de não estar cheia
		return _btree_insert_nonfull(root, pair, bt->order);
	}
}

/*
    Trata todos os casos de remoção de uma key na B-Tree.
    Realizada possíveis chamadas recursivas.

    Retorna um node_position indicando onde a chave estava localizada.
    ATENÇÃO: Pode não ser seguro acessar tal posição.
    Retorna node_position(NULL, -1) caso key não exista em bt.
 */
node_position _btree_remove_node(node_t *node, int key, int order) {
	int pos;
	if (_node_find_key(node, key, &pos)) {
		// Caso a chave está foi encontrada nesse nó
		if (node->is_leaf) {
			/*
			    Caso 1: Se a chave k está em um nó x e x é uma
			    folha, remova a chave k de x
			 */
			free(node->keys[pos]);
			node->n_keys--;

			_node_deslocate_keys_up(node, node, pos, node->n_keys, 0, 1);

			return _node_position_new(node, pos);
		}
		else {
			/*
			    Caso 2: Se a chave k está em um nó x e x é um nó
			    interno.
			 */
			node_t *left = node->children[pos];
			node_t *right = node->children[pos+1];
			if (left->n_keys >= order) {
				/*
				    a) Se o nó filho y que precede k no nó x
				    tem pelo menos t chaves, encontre o
				    predecessor k’ de k na subárvore
				    enraizada em y.
				    Remova k’, e substitua k por k’ em x.
				 */

				free(node->keys[pos]);

				node_position max = _node_find_max(left);
				pair_t *p = _pair_copy(max.node->keys[max.index]);

				node->keys[pos] = p;

				_btree_remove_node(left, p->key, order);
				return _node_position_new(node, pos);
			}
			else if (right->n_keys >= order) {
				/*
				    b) Caso simétrico ao 2a. Se o nó filho y que
				    precede k no nó x tem menos que t chaves,
				    examine o nó filho z que vem depois de
				    k no nó x.
				 */

				free(node->keys[pos]);

				node_position min = _node_find_min(right);
				pair_t *p = _pair_copy(min.node->keys[min.index]);
				node->keys[pos] = p;

				_btree_remove_node(right, p->key, order);
				return _node_position_new(node, pos);
			}
			else {
				/*
				    2c) Se ambos os nós filhos y e z possuem
				    apenas t-1 chaves, concatena-se k e todo o
				    conteúdo de z com y, de maneira que x perca
				    a chave k e o ponteiro para z
				 */

				left->keys[order-1] = node->keys[pos];

				_node_deslocate_keys_up(left, right, 0, order-1, order, 0);
				_node_deslocate_children_up(left, right, 0, order, order, 0);
				left->n_keys = 2*order - 1;

				_node_delete(right);

				_node_deslocate_keys_up(node, node, pos, node->n_keys-1, 0, 1);
				_node_deslocate_children_up(node, node, pos, node->n_keys-1, 1, 2);
				node->n_keys--;

				return _btree_remove_node(left, key, order);
			}
		}
	}
	else {
		if (node->is_leaf) {
			/*
			    key não foi encontrada em um nó folha => key não
			    pertence à árvore
			 */
			return _node_position_new(NULL, -1);
		}

		/*
		        Caso 3: se a chave k não está presente em um nó interno x,
		        determine a raiz x.ci da subárvore que deve conter k.
		 */

		node_t *next = node->children[pos];

		if (next->n_keys == order-1) {
			node_t *left = next;

			node_t *right;
			if (pos == node->n_keys) {
				node_t *tmp = left;
				left = node->children[pos-1];
				right = tmp;
			}
			else {
				right = node->children[pos+1];
			}

			if (left->n_keys >= order) {
				/*
					a-left) Se x.c[i] tiver apenas t-1 chaves, mas possui o irmão
					esquerdo imediato com pelo menos t chaves, mover uma chave de x
					para x.c[i].

					Mover para x uma chave do irmão imediato esquerdo de x.c[i].
					Mover os ponteiros associados para que apontem para os filhos
					corretos.
				 */

				node_position max = _node_find_max(left);
				pair_t *p = _pair_copy(max.node->keys[max.index]);

				_btree_remove_node(left, p->key, order);

				_btree_insert_nonfull(right, node->keys[pos-1], order);

				node->keys[pos-1] = p;

				return _btree_remove_node(right, key, order);
			}
			else if (right->n_keys >= order) {
				/*
					a-right) Se x.c[i] tiver apenas t-1 chaves, mas possui o irmão
					direito imediato com pelo menos t chaves, mover uma chave de x
					para x.c[i].

					Mover para x uma chave do irmão imediato direito de x.c[i].
					Mover os ponteiros associados para que apontem para os filhos
					corretos.
				 */

				node_position min = _node_find_min(right);
				pair_t *p = _pair_copy(min.node->keys[min.index]);

				_btree_remove_node(right, p->key, order);

				_btree_insert_nonfull(left, node->keys[pos-1], order);

				node->keys[pos-1] = p;

				return _btree_remove_node(left, key, order);
			}
			else {
				/*
					b) Se x.c[i] e ambos os seus irmãos imediatos tiverem t-1
					chaves, concatenar x.c[i] com um de seus irmãos.

					Essa concatenação envolve mover uma chave de x para o novo
					nó criado com a concatenação, para que ele se torne a chave
					mediana desse novo nó.
				 */

				left->keys[order-1] = node->keys[pos-1];
				_node_deslocate_keys_up(node, node, pos, node->n_keys-1, 0, 1);
				_node_deslocate_children_up(node, node, pos+1, node->n_keys, 0, 1);
				node->n_keys--;

				_node_deslocate_keys_up(left, right, 0, order-1, order, 0);
				_node_deslocate_children_up(left, right, 0, right->n_keys+1, order-1, 0);
				_node_delete(right);

				left->n_keys = 2*order-1;
				return _btree_remove_node(left, key, order);
			}
		}

		return _btree_remove_node(next, key, order);
	}
}

/*
    Remove a chave key na B-Tree bt.
    Chama a função _btree_remove_node() sobre a raiz de bt.
    ATENÇÃO: Pode não ser seguro acessar tal posição.
 */
node_position btree_remove(BTree* bt, int key) {

	assert(bt != NULL);

	node_position pos = _btree_remove_node(bt->root, key, bt->order);
	if (bt->root->n_keys == 0 && pos.node != NULL && pos.node != bt->root) {
		// A B-Tree deve diminuir de altura, pois a remoção fez com que
		// a raiz ficasse vazia. A nova raiz será o nó-filho da esquerda.

		bt->root = bt->root->children[0];
		assert(bt->root != NULL);
	}

	return pos;
}

/*
    Deleta uma B-Tree alocada na Stack.
    Utilizada como sub-procedimento na função btree_delete_h().
    NÃO tenta dar free na B-Tree: free(bt).
 */
void btree_delete_s(BTree *bt) {
	// Deleta uma B-Tree sem chamar free(tree),
	// mas sim removendo todas as chaves que ela possui
	while (bt->root->n_keys > 0) {
		btree_remove(bt, bt->root->keys[0]->key);
	}

	_node_delete(bt->root);
}

/*
    Deleta uma B-Tree alocada na Heap.
    EXECUTA um FREE na B-Tree: free(bt).
    Undefined behavior caso bt não tenha sido alocado com uma chamada à malloc().
 */
void btree_delete_h(BTree *bt) {
	// Deleta a B-Tree e todas as suas chaves-valores.
	// A B-Tree deve ter sido alocada com uma função malloc(),
	// caso contrário, tem-se comportamento indefinido
	btree_delete_s(bt);
	free(bt);
}



	//Estrutura Data
typedef struct data{
	int dia;
	int mes;
	int ano;
}TData;

/*
Forma_pagamento:1-dinheiro
				2-cheque
				3-boleto
				4-cartao
			tipo: entrada 0; saida 1;
*/

typedef struct entrada{
	TData data;
	int tipo;
	int forma_pagamento;
	double valor;
	double valor_parcela;
	int n_parcelas;
	int parcela_atual;
	char descr[50];
	int chave;
}TEntrada;

//Função de leitura de entrada (Credito ou Debito) 

TEntrada* lerEntrada(int tipo,int dia,int mes,int ano,double valor,int formapagamento,int nparcelas,double valorparcela,int parcelaatual,char descri[50]){
	TEntrada *ex = malloc(sizeof(TEntrada));
	ex->tipo = tipo;
	ex->data.dia = dia;
	ex->data.mes = mes;
	ex->data.ano = ano;
	ex->valor = valor;
	if(ex->tipo==1)
		ex->valor = -valor;
	ex->forma_pagamento = formapagamento;
	ex->n_parcelas = nparcelas;
	ex->valor_parcela = (valor/nparcelas);
	if(ex->tipo==1)
		ex->valor_parcela = -(ex->valor_parcela);
	ex->parcela_atual = parcelaatual;

	strcpy((ex->descr),descri);
	int data = (((dia*100)+mes)*100)+(ano%100);
	int chave1 = (data*1000)+1;
	ex->chave = chave1;
	return ex;
}

//Estrutura para guardar valores creditados e debitados

typedef struct financas{
	double valor_entrada;
	double valor_saida;
}TFinancas;

//Funçao que inicializa TFinanca

void financa_init(TFinancas *a){
	a->valor_entrada=0;
	a->valor_saida=0;
}

 //Funçao para retornar um ponteiro TFinanca
TFinancas* financas_new(){
	TFinancas *a = malloc(sizeof(TFinancas));
	financa_init(a);
	return a;
}

//Ponteiro global para a uma variavel TFinanca que irá guardar os valores creditados debitados.
    //Esse ponteiro é único, sendo sempre reaproveitado pela aplicação.
	
TFinancas *tf;

//Funçao que insere os valores na arvore
BTree* inserirEntrada(BTree *arvore){
	int tipo;
	printf("0 - Credito\n1 - Debito\n");
	printf("\n");
	printf(" Digite uma opcao: ");
	scanf("%d",&tipo);

	int dia,mes,ano;
	printf("\nDigite a data(DD/MM/AAAA):");
	scanf("%d/%d/%d",&dia,&mes,&ano);
	double valor;
	printf("\nValor:");
	scanf("%lf",&valor);


	int formapagamento;
	printf("\n1-Dinheiro\n2-Cheque\n3-Boleto\n4-Cartao\nForma de pagamento:");
	scanf("%d",&formapagamento);

	int nparcelas=1;
	if(formapagamento==4){
		printf("1 - A Vista\n2 - 2x\n3 - 3x\n4 - 4x\n5 - 5x\n6 - 6x\nParcelado em:");
		scanf("%d",&nparcelas);
	}
	double valorparcela = valor/nparcelas;
	int parcelaatual=1;
	char descri[50];
	getchar();
	printf("Descricao da entrada:");
	fgets(descri,100,stdin);

	int i;
	TEntrada *entrada;
	for(i=0;i<nparcelas;i++){
		entrada = lerEntrada(tipo,dia,mes,ano,valor,formapagamento,nparcelas,valorparcela,parcelaatual,descri);
		mes++;

		if(mes>12){
			mes=1;
			ano++;
		}
		parcelaatual++;
		node_position info = btree_insert(arvore, (entrada->chave), entrada);
		while(info.node == NULL){
				(entrada->chave)++;
				info = btree_insert(arvore, (entrada->chave), entrada);
			}
		}
	return arvore;
}


void listar_nos_geral(node_t *node, int level) {
	assert(node != NULL);

	int i;
	if (!node->is_leaf) {
		for (i = 0; i < node->n_keys+1; ++i) {
			listar_nos_geral(node->children[i], level+1);
		}
	}
	TEntrada *b;
	//printf("\tAlgum nó no nível %d possui %d chave(s): ", level, node->n_keys);
	for (i = 0; i < node->n_keys; ++i) {
		printf("Cod: %.9d.....", node->keys[i]->key);
		b = (node->keys[i]->value);
		if(!(b->tipo)){
			printf("Entrada.....");
			tf->valor_entrada = tf->valor_entrada + b->valor_parcela;
		}
		else {
			printf("Saida.......");
			tf->valor_saida = tf->valor_saida + b->valor_parcela;
		}
		printf("%.2d/%.2d/%d.....",(b->data.dia),(b->data.mes),(b->data.ano));
		printf("Valor: R$%.2lf.....",(b->valor));
		printf("Pagamento no ");
		switch(b->forma_pagamento){
			case 1: printf("Dinheiro.....");
					break;
			case 2: printf("Cheque.....");
					break;
			case 3: printf("Boleto.....");
					break;
			case 4: printf("Cartao.....");
					break;
		}
		if(b->forma_pagamento==4){
		printf("Valor da parcela: R$%.2lf.....",(b->valor_parcela));
		printf("Parcela atual: %d/%d.....",(b->parcela_atual),(b->n_parcelas));
	}
		printf("%s",(b->descr));
	}
}


void listar_nos_dia(node_t *node, int level,int d, int m, int a) {
	assert(node != NULL);
	int i;

	if (!node->is_leaf) {
		for (i = 0; i < node->n_keys+1; ++i) {
			listar_nos_dia(node->children[i], level+1,d,m,a);
		}
	}
	TEntrada *b;
	//printf("\tAlgum nó no nível %d possui %d chave(s): ", level, node->n_keys);
	for (i = 0; i < node->n_keys; ++i) {
		b = (node->keys[i]->value);
		if(((b->data.dia)==d)&&((b->data.mes)==m)&&((b->data.ano)==a)){

		printf("Cod: %.9d.....", node->keys[i]->key);
		if(!(b->tipo)){
			printf("Entrada.....");
			tf->valor_entrada = tf->valor_entrada + b->valor_parcela;
		}
		else {
			printf("Saida.......");
			tf->valor_saida = tf->valor_saida + b->valor_parcela;
		}
		printf("%.2d/%.2d/%d.....",(b->data.dia),(b->data.mes),(b->data.ano));
		printf("Valor: R$%.2lf.....",(b->valor));
		printf("Pagamento no ");
		switch(b->forma_pagamento){
			case 1: printf("Dinheiro.....");
					break;
			case 2: printf("Cheque.....");
					break;
			case 3: printf("Boleto.....");
					break;
			case 4: printf("Cartao.....");
					break;
		}
		if(b->forma_pagamento==4){
		printf("Valor da parcela: R$%.2lf.....",(b->valor_parcela));
		printf("Parcela atual: %d/%d.....",(b->parcela_atual),(b->n_parcelas));
	}
		printf("%s",(b->descr));
	}
	}
}

void listar_nos_mes(node_t *node, int level, int m, int a) {
	assert(node != NULL);
	int i;

	if (!node->is_leaf) {
		for (i = 0; i < node->n_keys+1; ++i) {
			listar_nos_mes(node->children[i], level+1,m,a);
		}
	}
	TEntrada *b;
	//printf("\tAlgum nó no nível %d possui %d chave(s): ", level, node->n_keys);
	for (i = 0; i < node->n_keys; ++i) {
		b = (node->keys[i]->value);
		if(((b->data.mes)==m)&&((b->data.ano)==a)){

		printf("Cod: %.9d.....", node->keys[i]->key);
		if(!(b->tipo)){
			printf("Entrada.....");
			tf->valor_entrada = tf->valor_entrada + b->valor_parcela;
		}
		else {
			printf("Saida.......");
			tf->valor_saida = tf->valor_saida + b->valor_parcela;
		}
		printf("%.2d/%.2d/%d.....",(b->data.dia),(b->data.mes),(b->data.ano));
		printf("Valor: R$%.2lf.....",(b->valor));
		printf("Pagamento no ");
		switch(b->forma_pagamento){
			case 1: printf("Dinheiro.....");
					break;
			case 2: printf("Cheque.....");
					break;
			case 3: printf("Boleto.....");
					break;
			case 4: printf("Cartao.....");
					break;
		}
		if(b->forma_pagamento==4){
		printf("Valor da parcela: R$%.2lf.....",(b->valor_parcela));
		printf("Parcela atual: %d/%d.....",(b->parcela_atual),(b->n_parcelas));
	}
		printf("%s",(b->descr));
	}
	}
}

void listar_nos_ano(node_t *node, int level, int a) {
	assert(node != NULL);
	int i;

	if (!node->is_leaf) {
		for (i = 0; i < node->n_keys+1; ++i) {
			listar_nos_ano(node->children[i], level+1,a);
		}
	}
	TEntrada *b;
	//printf("\tAlgum nó no nível %d possui %d chave(s): ", level, node->n_keys);
	for (i = 0; i < node->n_keys; ++i) {
		b = (node->keys[i]->value);
		if((b->data.ano)==a){

		printf("Cod: %.9d.....", node->keys[i]->key);
		if(!(b->tipo)){
			printf("Entrada.....");
			tf->valor_entrada = tf->valor_entrada + b->valor_parcela;
		}
		else {
			printf("Saida.......");
			tf->valor_saida = tf->valor_saida + b->valor_parcela;
		}
		printf("%.2d/%.2d/%d.....",(b->data.dia),(b->data.mes),(b->data.ano));
		printf("Valor: R$%.2lf.....",(b->valor));
		printf("Pagamento no ");
		switch(b->forma_pagamento){
			case 1: printf("Dinheiro.....");
					break;
			case 2: printf("Cheque.....");
					break;
			case 3: printf("Boleto.....");
					break;
			case 4: printf("Cartao.....");
					break;
		}
		if(b->forma_pagamento==4){
		printf("Valor da parcela: R$%.2lf.....",(b->valor_parcela));
		printf("Parcela atual: %d/%d.....",(b->parcela_atual),(b->n_parcelas));
	}
		printf("%s",(b->descr));
	}
	}
}

void listar_arvore_geral(BTree *bt) {
	assert(bt != NULL);
	listar_nos_geral(bt->root, 0);
	printf("\nValor Creditado: R$%.2lf\nValor Debitado: R$%.2lf\nTotal: R$%.2lf\n",tf->valor_entrada,tf->valor_saida,(tf->valor_entrada)+(tf->valor_saida));
	tf->valor_entrada=0;
	tf->valor_saida=0;
	getchar();
}

void listar_arvore_dia(BTree *bt) {
	assert(bt != NULL);
	int d,m,a;
	printf("\n Digite a data(DD/MM/AAAA):");
	scanf("%d/%d/%d",&d,&m,&a);
	listar_nos_dia(bt->root, 0,d,m,a);
	printf("\nValor Creditado: R$%.2lf\nValor Debitado: R$%.2lf\nTotal: R$%.2lf\n",tf->valor_entrada,tf->valor_saida,(tf->valor_entrada)+(tf->valor_saida));
	tf->valor_saida=0;
	 getchar();
}

void listar_arvore_mes(BTree *bt) {
	assert(bt != NULL);
	int d,m,a;
	printf("\n Digite o mes desejado(MM/AAAA):");
	scanf("%d/%d",&m,&a);
	listar_nos_mes(bt->root, 0,m,a);
	printf("\nValor Creditado: R$%.2lf\nValor Debitado: R$%.2lf\nTotal: R$%.2lf\n",tf->valor_entrada,tf->valor_saida,(tf->valor_entrada)+(tf->valor_saida));
	tf->valor_entrada=0;
	tf->valor_saida=0;
	 getchar();
}

void listar_arvore_ano(BTree *bt) {
	assert(bt != NULL);
	int d,m,a;
	printf("\n Digite o ano desejado(AAAA):");
	scanf("%d",&a);
	listar_nos_ano(bt->root, 0,a);
	printf("\nValor Creditado: R$%.2lf\nValor Debitado: R$%.2lf\nTotal: R$%.2lf\n",tf->valor_entrada,tf->valor_saida,(tf->valor_entrada)+(tf->valor_saida));
	tf->valor_entrada=0;
	tf->valor_saida=0;
	 getchar();
}
void _btree_dfs_node(node_t *node, int level) {
	assert(node != NULL);

	int i;
	if (!node->is_leaf) {
		for (i = 0; i < node->n_keys+1; ++i) {
			_btree_dfs_node(node->children[i], level+1);
		}
	}

	printf("\tAlgum no no nivel %d possui %d chave(s): ", level, node->n_keys);
	for (i = 0; i < node->n_keys; ++i) {
		if (i != 0) printf(" ");
		printf("%d", node->keys[i]->key);
	}
	printf("\n");
}

void btree_dfs(BTree *bt) {
	assert(bt != NULL);
	_btree_dfs_node(bt->root, 0);
}

/*
    Define uma forma de limpar a tela do terminal,
    tanto no Linux quanto no Windows.
 */
#ifdef __linux__
    #define CLEAR() (system("clear"))
#elif _WIN32
    #define CLEAR() (system("cls"))
#endif
#define MSG_LEN 100

/*
    Imprime uma mensagem de cabeçalho
    compartilhada por todas as screens
 */
void print_header();

/*
    Tela de Home
    Permite acessar a screen de criação de B-Tree e a screen Sobre.
    Permite sair da aplicação.
 */
int home_screen();
/*
    Informações básicas sobre o projeto B-Tree e essa DEMO
 */
int about_screen();
/*
    Tela para definir a ordem da B-Tree a ser criada
 */
int run_screen();
/*
    Tela onde as operações INSERÇÃO, REMOÇÃO, PESQUISA, IMPRESSÃO serão executadas
 */
int running_screen();
/*
    Função auxiliar para gerar uma mensagem de "despedida"
 */
void goodbye();
/*
    A ordem das 3 primeiras screens deve ser RUN, ABOUT, EXIT.
    Os valores atribuídos à elas são utilizados na lógica interna do menu.
 */
typedef enum {
	RUN, ABOUT, EXIT, HOME, RUNNING
} screen;

/*
    Indica uma mensagem que será exibida no topo da próxima screen.
    É uma forma de comunicar mensagens ao usuário.
    O valor default (quando não há mensagens para exibir) deve ser o caracter '-'.
 */
char msg[MSG_LEN];
/*
    Ponteiro global para a B-Tree que será instanciada.
    Esse ponteiro é único, sendo sempre reaproveitado pela aplicação.
 */
BTree *tree;
	//Ponteiro global para a uma variavel TEntrada que será instanciada.
    //Esse ponteiro é único, sendo sempre reaproveitado pela aplicação.
TEntrada *a;
	
/*
    Contador de quantas keys foram inseridas na B-Tree atual.
    É reiniciado sempre que a B-Tree é deletada.
 */
int n_keys;


int home_screen() {
	int opt;
	int n_items = 3;
	// Itens no menu
	char list[3][12] = { "Nova Arvore", "Sobre", "Sair" };

	print_header();

	int i;
	for (i = 0; i < n_items; ++i) {
		printf(" %d) %s\n", i+1, list[i]);
	}
	printf("\n");
	printf(" Digite uma opcao: ");
	scanf("%d%*c", &opt);

	if (opt < 1 || opt > n_items) {
		snprintf(msg, MSG_LEN, " Opcao invalida.");
		return HOME;
	}

	return opt-1;
}

int about_screen() {
	print_header();
	printf( "---------------------------------------------------------\n" \
	        "--       Trabalho apresentado na disciplina AED2       --\n" \
	        "--            ministrada pelo Prof. Edson              --\n" \
	        "--                    UFAM                             --\n" \
	        "---------------------------------------------------------\n");
	printf("\n");

	printf(" Pressione ENTER para voltar ao menu. ");
	getchar();

	return HOME;
}

int run_screen() {
	print_header();

	int order;

	printf("  Um no de uma Arvore-B de ordem T deve ter no minimo T-1\n" \
	       "               e no maximo 2*T filhos\n");
	printf("\n");
	printf(" Digite a ordem T da sua B-Tree: ");
	scanf("%d%*c", &order);

	// Única criação de uma B-Tree
	tree = btree_new(order);
	snprintf(msg, MSG_LEN, " B-Tree de ordem %d alocada.", order);
	tf = financas_new();

	return RUNNING;
}

int running_screen() {
	int n_items = 5;
	// Itens no menu
	char list[5][15] = { "Lancamentos", "Consultas", "Balanco Geral", "Arvore", "Excluir Arvore" };
	int opt;

	print_header();

	int i;
	for (i = 0; i < n_items; ++i) {
		printf(" %d) %s\n", i+1, list[i]);
	}

	printf("\n");
	printf(" Digite uma opcao: ");
	scanf("%d%*c", &opt);
	printf("\n");
	int op;
	node_position pos;
	switch (opt) {
	case 1:                 // Lançamentos
			printf("1 - Inserir\n2 - Remover\n");
			printf("\n");
			printf(" Digite uma opcao: ");
			scanf("%d", &op);
			printf("\n");
			switch(op){
				//inserir
				case 1: inserirEntrada(tree);
						snprintf(msg, MSG_LEN, " Insercao realizada com sucesso.");
						n_keys++;
						break;
				//remover
				case 2:	printf(" Digite uma CHAVE para remover: ");
						scanf("%d", &op);
						pos = btree_remove(tree, op);
						if (pos.node == NULL) {
							snprintf(msg, MSG_LEN, " A CHAVE %d nao foi encontrada na B-Tree.", op);
						}
						else {
							snprintf(msg, MSG_LEN, " A CHAVE %d foi removida com sucesso.", op);
						}
						break;
			}
		break;
	case 2:                 // Consultas
		printf("\n");
		printf("1 - Por codigo\n2 - Por dia\n3 - Por mes\n4 - Por ano\n");
		printf("\n");
		printf(" Digite uma opcao: ");
		scanf("%d", &op);
		int key;
		switch(op){
			case 1: printf(" Digite uma chave: ");
					scanf("%d",&key);
					printf("\n");
					pos = btree_find(tree, key);
					if (pos.node == NULL) {
						snprintf(msg, MSG_LEN, " A CHAVE %d nao foi encontrada na B-Tree.", op);
					}
					else {
						getchar();
						printf(" Cod: %d.....", ((pos.node)->keys[pos.index])->key);
						a = (((pos.node)->keys[pos.index])->value);
						if(!(a->tipo))
						printf("Entrada.....");
						else printf("Saida.......");
						printf("%d/%d/%d.....",(a->data.dia),(a->data.mes),(a->data.ano));
						printf("Valor: R$%.2lf.....",(a->valor));
						printf("Pagamento no ");
						switch(a->forma_pagamento){
							case 1: printf("Dinheiro.....");
									break;
							case 2: printf("Cheque.....");
									break;
							case 3: printf("Boleto.....");
									break;
							case 4: printf("Cartao.....");
									break;
						}
						if(a->forma_pagamento==4){
							printf("Valor da parcela: R$%.2lf.....",(a->valor_parcela));
							printf("Parcela atual: %d/%d.....",(a->parcela_atual),(a->n_parcelas));
						}
						printf("%s",(a->descr));
						}
						printf(" Pressione ENTER para continuar. ");
						getchar();
						break;
				case 2: listar_arvore_dia(tree);
						printf(" Pressione ENTER para continuar. ");
						getchar();
						break;
				case 3: listar_arvore_mes(tree);
						printf(" Pressione ENTER para continuar. ");
						getchar();
						break;
				case 4: listar_arvore_ano(tree);
						printf(" Pressione ENTER para continuar. ");
						getchar();
						break;
			}


		break;
	case 3:                 //Balanço Geral
		listar_arvore_geral(tree);
		break;
	case 4:                 //Arvore
		printf(" Mostrando a arvore:\n");
		btree_dfs(tree);
		printf(" Pressione ENTER para continuar. ");
		getchar();
		break;
	case 5:                 // Voltar/Sair
		snprintf(msg, MSG_LEN, " Deletando B-Tree e voltando ao menu");
		goodbye();
		// Única deleção de uma B-Tree
		btree_delete_h(tree);
		return HOME;
		break;
	default:
		snprintf(msg, MSG_LEN, " Opcao invalida");
		return RUNNING;
		break;
	}

	return RUNNING;
}

void print_header() {
	CLEAR();
	printf( "---------------------------------------------------------\n" \
	        "--                    Arvore-B                         --\n" \
	        "--                  Desenvolvido por:                  --\n" \
	        "--      Jean Cleison Braga Guimaraes - 21601227        --\n" \
	        "---------------------------------------------------------\n");

	printf("\n");

	if (strcmp(msg, "-")) {
		printf("%s\n\n", msg);
		snprintf(msg, MSG_LEN, "-");
	}
}

void goodbye() {
	// Apenas uma animação simples de "loading"
	print_header();
	int i;
	printf(" ");
	for (i = 0; i < 4; ++i) {
		printf(".");
		fflush(stdout);
		usleep(500000);
	}

	CLEAR();
}

int main() {
	screen next_screen = HOME;
	snprintf(msg, MSG_LEN, "-");
	n_keys = 0;

	while (next_screen != EXIT) {
		switch (next_screen) {
		case HOME:
			next_screen = home_screen();
			break;
		case ABOUT:
			next_screen = about_screen();
			break;
		case RUN:
			next_screen = run_screen();
			break;
		case RUNNING:
			next_screen = running_screen();
			break;
		case EXIT:
			break;
		}
	}

	snprintf(msg, MSG_LEN, " Saindo da aplicacao");
	goodbye();

	return 0;
}
