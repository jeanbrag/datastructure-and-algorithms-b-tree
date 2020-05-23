#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

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

//Ponteiro global para a uma variavel TFinanca que irá guardar os valores creditados debitados.
    //Esse ponteiro é único, sendo sempre reaproveitado pela aplicação.
	
TFinancas *tf;
	
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
