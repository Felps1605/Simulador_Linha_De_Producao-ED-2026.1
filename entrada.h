#ifndef ENTRADA_H
#define ENTRADA_H

#include <stdio.h>
#include "struct.h"

// ---------------------- Criação ----------------------
etapa* criarEtapa(int id, char *nome, int capacidade_max);
atividade* criarAtividade(int id, char *nome, int tempo, float failrate, int qtd_uf);

// ---------------------- Ligação ----------------------
void adicionarAtividade(etapa *e, atividade *a);
void adicionarEtapa(etapas *linha, etapa *e);

// ---------------------- Leitura ----------------------
// Função única que lê todo o arquivo de entrada, valida os dados
// e preenche a simulação completa
void lerEntrada(FILE *arquivo, simulacao *sim);

#endif
