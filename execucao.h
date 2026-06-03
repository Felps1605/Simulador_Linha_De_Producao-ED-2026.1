#ifndef EXECUCAO_H
#define EXECUCAO_H

#include "structs.h"


void enfileirar(produto *p, fila **f);

void empilhar(produto *p, pilha **pp);

void criar_produto(simulacao *s);

produto *desenfileirar(fila *f);

void mostrar_fila(fila *f);

void mostrar_pilha(pilha *p, simulacao *s);

void falhar_produto(produto *p, float failrate);

void mostrar_atividades(atividade *primeira);

void mostrar_etapas(etapas *e);

void pegar_produto_etapa(etapa *e, fila *fila_entrada);

void pegar_produto_atividade(atividade *a);

int verificar_defeitos(produto *p, simulacao *s);

void avancar_produto(atividade *a, produto *p, simulacao *s);

void atualizar_atividade(atividade *a, simulacao *sim);

void envelhecer_atividade(atividade *a);

void liberar_fila(fila *f);

void liberar_atividade(atividade *a);

void liberar_atividades(etapa *e);

void liberar_etapas(etapas *e);

void liberar_pilha(pilha *p);

void encerrar_simulacao(simulacao *s);

etapa *buscar_etapa(etapas *e, int id);

atividade *buscar_atividade(etapas *e, int id);

void contagem_regressiva( const char *mensagem);

void opcoes(simulacao *s);

int linha_vazia(simulacao *s);

int ha_produtos_para_entrar_etapa(etapa *e, simulacao *s);

int ha_produtos_para_entrar_atividade(atividade *a);

void envelhecer_produtos(simulacao *s);

void saidas(simulacao *s);

void entradas(simulacao *s);

void popular(simulacao *s);

void simular(simulacao *s);

#endif