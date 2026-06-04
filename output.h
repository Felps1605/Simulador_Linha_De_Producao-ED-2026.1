#ifndef OUTPUT_H
#define OUTPUT_H

#include "structs.h"

struct tempos{
    float tempo_em_fila_de_atividades;
    float tempo_em_fila_de_prontos;
};
typedef struct tempos tempos;

int tempo_minimo(etapa *e);

int buscar_tempo_na_etapa(produto *p, int id);

void setar_tempo_em_fila_na_etapa(tempos * t, produto * p, int id);

int buscar_tempo_em_fila_nas_atividades(evento_etapa *e);

float tempo_medio_produtos_finalizados(etapa *e, pilha *p);

int tempo_maximo_produtos_finalizados(etapa *e, pilha *p);

tempos tempo_medio_em_fila_finalizados(etapa *e, pilha * p);

void mostrar_relatorio_etapas(etapas *e, pilha *p);



#endif