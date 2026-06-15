#ifndef OUTPUT_H
#define OUTPUT_H

#include "structs.h"

void relatorio_simulacao(simulacao *s);

void imprimir_historico_produto(simulacao * s, produto * p);

void imprimir_metadados(simulacao *s);

void imprimir_relatorio_etapas(simulacao *s);

void imprimir_relatorio_atividades(simulacao *s);

void imprimir_relatorio_produtos(simulacao *s);

void preencher_resumos(resumo_simulacao *rs, pilha *produtos);

void inicializar_resumos(simulacao *s);

void liberar_resumos(simulacao *s);

#endif