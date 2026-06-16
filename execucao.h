#ifndef EXECUCAO_H
#define EXECUCAO_H

#include "structs.h"

void opcoes_finais(simulacao *s);

void simular(simulacao *s);

void mostrar_pilha(pilha * p, simulacao * s);

void limpar_buffer();

void encerrar_simulacao(simulacao *s);

#endif