#ifndef CONSTRUCAO_H
#define CONSTRUCAO_H

#include "structs.h"

void inicializar_simulacao(simulacao *s);
void criar_slots(atividade *a);
void criar_atividade(etapa *dona, int indice, int capacidade);
void criar_etapa(simulacao *s);

#endif