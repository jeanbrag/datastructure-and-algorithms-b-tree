#include "functions.h"
#include <strings.h>

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
