#ifndef OUTPUT_H
#define OUTPUT_H

#include "structs.h"

void relatorio_simulacao(simulacao *s);

void imprimir_historico_produto(simulacao * s, produto * p);

void mostrar_pilha(pilha *p, simulacao *s);

void imprimir_metadados(simulacao *s);

void imprimir_relatorio_etapas(simulacao *s);

void imprimir_relatorio_atividades(simulacao *s);

void imprimir_relatorio_produtos(simulacao *s);

#endif