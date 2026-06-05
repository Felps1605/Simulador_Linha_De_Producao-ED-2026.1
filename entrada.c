#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------- Estruturas ----------------------

typedef struct Atividade {
    int id;
    int tempo;
    float taxaFalha;
    char nome[50];
    struct Atividade *prox;
} Atividade;

typedef struct Etapa {
    int id;
    int capacidade;
    float taxaFalhaInicial;
    char nome[50];
    Atividade *atividades;
    struct Etapa *prox;
    struct Etapa *ant;
} Etapa;

typedef struct Simulacao {
    char nomeSimulacao[50];
    int semente;
    int limiteTempo;
    int qtdProdutos;
    int taxaProdutosSeg;
    char nomeProduto[50];
    Etapa *linhaProducao;
} Simulacao;

// ---------------------- Funções auxiliares ----------------------

Etapa* criarEtapa(int id, int capacidade, float taxaFalha, char *nome) {
    Etapa *nova = malloc(sizeof(Etapa));
    if (!nova) {
        printf("Erro: falha ao alocar memória para Etapa.\n");
        exit(1);
    }
    nova->id = id;
    nova->capacidade = capacidade;
    nova->taxaFalhaInicial = taxaFalha;
    strcpy(nova->nome, nome);
    nova->atividades = NULL;
    nova->prox = NULL;
    nova->ant = NULL;
    return nova;
}

Atividade* criarAtividade(int id, int tempo, float taxaFalha, char *nome) {
    Atividade *nova = malloc(sizeof(Atividade));
    if (!nova) {
        printf("Erro: falha ao alocar memória para Atividade.\n");
        exit(1);
    }
    nova->id = id;
    nova->tempo = tempo;
    nova->taxaFalha = taxaFalha;
    strcpy(nova->nome, nome);
    nova->prox = NULL;
    return nova;
}

void adicionarAtividade(Etapa *etapa, Atividade *atividade) {
    if (etapa->atividades == NULL) {
        etapa->atividades = atividade;
    } else {
        Atividade *aux = etapa->atividades;
        while (aux->prox != NULL) aux = aux->prox;
        aux->prox = atividade;
    }
}

void adicionarEtapa(Simulacao *sim, Etapa *etapa) {
    if (sim->linhaProducao == NULL) {
        sim->linhaProducao = etapa;
    } else {
        Etapa *aux = sim->linhaProducao;
        while (aux->prox != NULL) aux = aux->prox;
        aux->prox = etapa;
        etapa->ant = aux;
    }
}

// ---------------------- Funções de validação ----------------------

void validarSimulacao(Simulacao *sim) {
    if (sim->semente < 0 || sim->limiteTempo <= 0) {
        printf("Erro: valores inválidos em SIMULACAO. Ajuste o arquivo.\n");
        exit(1);
    }
    if (sim->qtdProdutos <= 0 || sim->taxaProdutosSeg <= 0) {
        printf("Erro: valores inválidos em PRODUTOS. Ajuste o arquivo.\n");
        exit(1);
    }
}

void validarEtapa(Etapa *etapa) {
    if (etapa->id < 0 || etapa->capacidade < 0 ||
        etapa->taxaFalhaInicial < 0.0 || etapa->taxaFalhaInicial > 1.0) {
        printf("Erro: dados inválidos na ETAPA %d (%s). Ajuste o arquivo.\n",
               etapa->id, etapa->nome);
        exit(1);
    }
}

void validarAtividade(Atividade *atividade) {
    if (atividade->id < 0 || atividade->tempo <= 0 ||
        atividade->taxaFalha < 0.0 || atividade->taxaFalha > 1.0) {
        printf("Erro: dados inválidos na ATIVIDADE %d (%s). Ajuste o arquivo.\n",
               atividade->id, atividade->nome);
        exit(1);
    }
}

// ---------------------- Programa principal ----------------------

int main() {
    FILE *arquivo;
    char linha[200];

    Simulacao *sim = malloc(sizeof(Simulacao));
    if (!sim) {
        printf("Erro: falha ao alocar memória para Simulacao.\n");
        return 1;
    }
    sim->linhaProducao = NULL;

    arquivo = fopen("entrada.txt", "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo.\n");
        return 1;
    }

    // Ignora primeira linha
    fgets(linha, sizeof(linha), arquivo);

    // SIMULACAO
    fgets(linha, sizeof(linha), arquivo);
    sscanf(linha, "SIMULACAO %49s %d %d", sim->nomeSimulacao, &sim->semente, &sim->limiteTempo);

    // PRODUTOS
    fgets(linha, sizeof(linha), arquivo);
    sscanf(linha, "PRODUTOS %d %d %49s", &sim->qtdProdutos, &sim->taxaProdutosSeg, sim->nomeProduto);

    validarSimulacao(sim);

    // LINHA_PRODUCAO
    int qtdEtapas;
    fgets(linha, sizeof(linha), arquivo);
    sscanf(linha, "LINHA_PRODUCAO %d", &qtdEtapas);

    if (qtdEtapas <= 0) {
        printf("Erro: quantidade de etapas inválida. Ajuste o arquivo.\n");
        exit(1);
    }

    // ETAPAS
    for (int i = 0; i < qtdEtapas; i++) {
        int id, qtdAtividades, capacidade;
        float taxaFalha;
        char nomeEtapa[50];

        fgets(linha, sizeof(linha), arquivo);
        sscanf(linha, "ETAPA %d %d %d %f %49s", &id, &qtdAtividades, &capacidade, &taxaFalha, nomeEtapa);

        Etapa *etapa = criarEtapa(id, capacidade, taxaFalha, nomeEtapa);
        validarEtapa(etapa);

        for (int j = 0; j < qtdAtividades; j++) {
            int idA, tempo;
            float taxaFalhaA;
            char nomeAtividade[50];

            fgets(linha, sizeof(linha), arquivo);
            sscanf(linha, "ATIVIDADE %d %d %f %49s", &idA, &tempo, &taxaFalhaA, nomeAtividade);

            Atividade *atividade = criarAtividade(idA, tempo, taxaFalhaA, nomeAtividade);
            validarAtividade(atividade);
            adicionarAtividade(etapa, atividade);
        }

        adicionarEtapa(sim, etapa);
    }

    fclose(arquivo);

    // ---------------------- TESTE ----------------------
    printf("SIMULACAO: %s\n", sim->nomeSimulacao);
    printf("Semente: %d | Limite tempo: %d\n", sim->semente, sim->limiteTempo);
    printf("Produtos: %d | Taxa: %d/s | Nome: %s\n", sim->qtdProdutos, sim->taxaProdutosSeg, sim->nomeProduto);

    Etapa *e = sim->linhaProducao;
    while (e) {
        printf("\nETAPA %d - %s (Capacidade %d, Falha %.2f)\n", e->id, e->nome, e->capacidade, e->taxaFalhaInicial);
        Atividade *a = e->atividades;
        while (a) {
            printf("  ATIVIDADE %d - %s (Tempo %d, Falha %.2f)\n", a->id, a->nome, a->tempo, a->taxaFalha);
            a = a->prox;
        }
        e = e->prox;
    }

    return 0;
}
