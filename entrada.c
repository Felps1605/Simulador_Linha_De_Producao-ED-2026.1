#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entrada.h"

etapa* criarEtapa(int id, char *nome, int capacidade_max) {
    etapa *nova = malloc(sizeof(etapa));
    if (!nova) { printf("Erro: falha ao alocar memória para etapa.\n"); exit(1); }
    nova->id = id;
    strcpy(nova->nome, nome);
    nova->capacidade_max = capacidade_max;
    nova->ocupacao = 0;
    nova->primeira_atividade = NULL;
    nova->ultima_atividade = NULL;
    nova->proxima_etapa = NULL;
    nova->etapa_anterior = NULL;
    nova->f = NULL;
    nova->falhas = 0;
    nova->qtd_produtos_concluidos = 0;
    nova->qtd_produtos_entraram = 0;
    return nova;
}

atividade* criarAtividade(int id, char *nome, int tempo, float failrate, int qtd_uf) {
    atividade *nova = malloc(sizeof(atividade));
    if (!nova) { printf("Erro: falha ao alocar memória para atividade.\n"); exit(1); }
    nova->id = id;
    strcpy(nova->nome, nome);
    nova->tempo_de_processamento = tempo;
    nova->failrate = failrate;
    nova->qtd_uf = qtd_uf;
    nova->capacidade_max = qtd_uf;
    nova->ocupacao = 0;
    nova->slots = NULL;
    nova->proxima_atividade = NULL;
    nova->f = NULL;
    return nova;
}

void adicionarAtividade(etapa *e, atividade *a) {
    if (e->primeira_atividade == NULL) {
        e->primeira_atividade = a;
        e->ultima_atividade = a;
    } else {
        e->ultima_atividade->proxima_atividade = a;
        e->ultima_atividade = a;
    }
    a->etapa_dona = e;
}

void adicionarEtapa(etapas *linha, etapa *e) {
    if (linha->primeira_etapa == NULL) {
        linha->primeira_etapa = e;
        linha->ultima_etapa = e;
    } else {
        linha->ultima_etapa->proxima_etapa = e;
        e->etapa_anterior = linha->ultima_etapa;
        linha->ultima_etapa = e;
    }
}

void lerEntrada(FILE *arquivo, simulacao *sim) {
    char linha[200];

    // Ignora primeira linha
    if (!fgets(linha, sizeof(linha), arquivo)) {
        printf("Erro: arquivo vazio.\n");
        exit(1);
    }

    // SIMULACAO
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "SIMULACAO %99s %d %d", sim->nome_cenario, &sim->semente, &sim->max_ticks) != 3) {
        printf("Erro: linha SIMULACAO inválida.\n");
        exit(1);
    }
    if (sim->semente < 0 || sim->max_ticks <= 0) {
        printf("Erro: valores inválidos em SIMULACAO.\n");
        exit(1);
    }

    // PRODUTOS
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "PRODUTOS %d %d %99s", &sim->n_produtos_total, &sim->vazao, sim->modelo_produto) != 3) {
        printf("Erro: linha PRODUTOS inválida.\n");
        exit(1);
    }
    if (sim->n_produtos_total <= 0 || sim->vazao <= 0) {
        printf("Erro: valores inválidos em PRODUTOS.\n");
        exit(1);
    }

    sim->linha = malloc(sizeof(etapas));
    sim->linha->primeira_etapa = NULL;
    sim->linha->ultima_etapa = NULL;

    // LINHA_PRODUCAO
    int qtdEtapas;
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "LINHA_PRODUCAO %d", &qtdEtapas) != 1 || qtdEtapas <= 0) {
        printf("Erro: linha LINHA_PRODUCAO inválida.\n");
        exit(1);
    }

    for (int i = 0; i < qtdEtapas; i++) {
        int id, qtdAtividades, capacidade;
        float taxaFalha;
        char nomeEtapa[50];

        if (!fgets(linha, sizeof(linha), arquivo) ||
            sscanf(linha, "ETAPA %d %d %d %f %49s", &id, &qtdAtividades, &capacidade, &taxaFalha, nomeEtapa) != 5) {
            printf("Erro: linha ETAPA inválida.\n");
            exit(1);
        }
        if (id < 0 || capacidade < 0 || taxaFalha < 0.0 || taxaFalha > 1.0 || qtdAtividades <= 0) {
            printf("Erro: dados inválidos na ETAPA %d.\n", id);
            exit(1);
        }

        etapa *e = criarEtapa(id, nomeEtapa, capacidade);

        for (int j = 0; j < qtdAtividades; j++) {
            int idA, tempo, qtdUF;
            float taxaFalhaA;
            char nomeAtividade[50];

            if (!fgets(linha, sizeof(linha), arquivo) ||
                sscanf(linha, "ATIVIDADE %d %d %f %d %49s", &idA, &tempo, &taxaFalhaA, &qtdUF, nomeAtividade) != 5) {
                printf("Erro: linha ATIVIDADE inválida.\n");
                exit(1);
            }
            if (idA < 0 || tempo <= 0 || taxaFalhaA < 0.0 || taxaFalhaA > 1.0 || qtdUF <= 0) {
                printf("Erro: dados inválidos na ATIVIDADE %d.\n", idA);
                exit(1);
            }

            atividade *a = criarAtividade(idA, nomeAtividade, tempo, taxaFalhaA, qtdUF);
            adicionarAtividade(e, a);
        }

        adicionarEtapa(sim->linha, e);
    }
}
