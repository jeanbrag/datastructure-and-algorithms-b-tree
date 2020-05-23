#include "btree.h"
#include "btree_tools.h"

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


//Estrutura para guardar valores creditados e debitados

typedef struct financas{
	double valor_entrada;
	double valor_saida;
}TFinancas;

TEntrada* lerEntrada(int tipo,int dia,int mes,int ano,double valor,int formapagamento,int nparcelas,double valorparcela,int parcelaatual,char descri[50]);
TFinancas* financas_new();
BTree* inserirEntrada(BTree *arvore);

void financa_init(TFinancas *a);
void listar_nos_geral(node_t *node, int level);
void listar_nos_dia(node_t *node, int level,int d, int m, int a);
void listar_nos_mes(node_t *node, int level, int m, int a);
void listar_nos_ano(node_t *node, int level, int a);
void listar_arvore_geral(BTree *bt);
void listar_arvore_dia(BTree *bt);
void listar_arvore_mes(BTree *bt);
void listar_arvore_ano(BTree *bt);