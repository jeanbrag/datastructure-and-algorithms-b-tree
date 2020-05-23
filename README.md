# Sistema de Fluxo Financeiro
## Implementação 

* O programa foi construído para um pequeno empreendedor, com o objetivo de
controlar o fluxo financeiro de uma loja, afim de facilitar a administração.
* Este pode fazer lançamentos de crédito/débito, apresentar relatórios diários,
mensais ou anuais mostrando os valores de entrada/saída e lucros/prejuízos.
* Todo o sistema utiliza de uma árvore-B para armazenar os dados. Com a
possibilidade de fazer inserção, remoção na mesma.
* O programa é baseado no uso de chaves, que são geradas na hora da inserção
do lançamento.
* A chave gerada é um número de 9 dígitos, sendo esta sua composição.
* 1º e 2º dígitos: Dia do lançamento.
* 3º e 4º dígitos: Mês do lançamento.
* 5º e 6º dígitos: Ano do lançamento.
* 7º, 8º e 9º dígitos: Número do lançamento no dia.
* Ex: 060219023
* Para o código acima, o algoritmo entende que este é o 23o lançamento do dia 06/02/2019
* O código é usado tanto para consulta de dados, quanto para remoção. Por isso é imprescindível que o usuário saiba fazer o uso corretamente desta informação.
