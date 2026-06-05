#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"

// ---------------------- Funções auxiliares ----------------------

etapa* criarEtapa(int id, char *nome, int capacidade_max) {
    etapa *nova = malloc(sizeof(etapa));
    if (!nova) { printf("Erro ao alocar memória para etapa.\n"); exit(1); }
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
    if (!nova) { printf("Erro ao alocar memória para atividade.\n"); exit(1); }
    nova->id = id;
    strcpy(nova->nome, nome);
    nova->tempo_de_processamento = tempo;
    nova->failrate = failrate;
    nova->qtd_uf = qtd_uf;
    nova->capacidade_max = qtd_uf; // simplificação
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

// ---------------------- Leitura do arquivo ----------------------

void lerSimulacao(FILE *arquivo, simulacao *sim) {
    char linha[200];
    fgets(linha, sizeof(linha), arquivo); // ignora primeira linha
    fgets(linha, sizeof(linha), arquivo);
    sscanf(linha, "SIMULACAO %99s %d %d", sim->nome_cenario, &sim->semente, &sim->max_ticks);

    fgets(linha, sizeof(linha), arquivo);
    sscanf(linha, "PRODUTOS %d %d %99s", &sim->n_produtos_total, &sim->vazao, sim->modelo_produto);

    sim->tick_atual = 0;
    sim->produtos_criados = 0;
    sim->produtos_concluidos = 0;
    sim->falhas_totais = 0;
    sim->linha = malloc(sizeof(etapas));
    sim->linha->primeira_etapa = NULL;
    sim->linha->ultima_etapa = NULL;
}

void lerLinhaProducao(FILE *arquivo, simulacao *sim) {
    char linha[200];
    int qtdEtapas;
    fgets(linha, sizeof(linha), arquivo);
    sscanf(linha, "LINHA_PRODUCAO %d", &qtdEtapas);

    for (int i = 0; i < qtdEtapas; i++) {
        int id, qtdAtividades, capacidade;
        float taxaFalha;
        char nomeEtapa[50];

        fgets(linha, sizeof(linha), arquivo);
        sscanf(linha, "ETAPA %d %d %d %f %49s", &id, &qtdAtividades, &capacidade, &taxaFalha, nomeEtapa);

        etapa *e = criarEtapa(id, nomeEtapa, capacidade);

        for (int j = 0; j < qtdAtividades; j++) {
            int idA, tempo, qtdUF;
            float taxaFalhaA;
            char nomeAtividade[50];

            fgets(linha, sizeof(linha), arquivo);
            sscanf(linha, "ATIVIDADE %d %d %f %d %49s", &idA, &tempo, &taxaFalhaA, &qtdUF, nomeAtividade);

            atividade *a = criarAtividade(idA, nomeAtividade, tempo, taxaFalhaA, qtdUF);
            adicionarAtividade(e, a);
        }

        adicionarEtapa(sim->linha, e);
    }
}

// ---------------------- Programa principal ----------------------

int main() {
    FILE *arquivo = fopen("entrada.txt", "r");
    if (!arquivo) { printf("Erro ao abrir arquivo.\n"); return 1; }

    simulacao *sim = malloc(sizeof(simulacao));
    if (!sim) { printf("Erro ao alocar memória para simulação.\n"); return 1; }

    lerSimulacao(arquivo, sim);
    lerLinhaProducao(arquivo, sim);

    fclose(arquivo);

    // Teste: imprimir
    printf("SIMULACAO: %s\n", sim->nome_cenario);
    printf("Semente: %d | Max ticks: %d\n", sim->semente, sim->max_ticks);
    printf("Produtos: %d | Vazao: %d/s | Modelo: %s\n", sim->n_produtos_total, sim->vazao, sim->modelo_produto);

    etapa *e = sim->linha->primeira_etapa;
    while (e) {
        printf("\nETAPA %d - %s (Capacidade %d)\n", e->id, e->nome, e->capacidade_max);
        atividade *a = e->primeira_atividade;
        while (a) {
            printf("  ATIVIDADE %d - %s (Tempo %d, Falha %.2f, UF %d)\n",
                   a->id, a->nome, a->tempo_de_processamento, a->failrate, a->qtd_uf);
            a = a->proxima_atividade;
        }
        e = e->proxima_etapa;
    }

    return 0;
}
