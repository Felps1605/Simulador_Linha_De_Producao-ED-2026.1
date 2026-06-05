#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // necessário para usar time()
#include "entrada.h"

// Função para inicializar a simulação com valores padrão
void inicializar_simulacao(simulacao *s)
{
    s->fila_entrada = NULL;
    s->lixo = NULL;
    s->concluidos = NULL;
    s->linha = NULL;
    s->produto_id = 1;
    s->etapa_id = 1;
    s->produtos_criados = 0;
    s->MODO_MANUAL = 0; // inicia em modo manual

    // Valores padrão (podem ser sobrescritos pelo arquivo)
    s->vazao = 2;
    s->n_produtos_total = 20;
    s->max_ticks = 100000;
    s->semente = time(NULL); // semente aleatória baseada no tempo atual
}

// Funções auxiliares
void criar_slots(atividade *a)
{
    a->slots = malloc(sizeof(slot) * a->capacidade_max);
    if (!a->slots)
    {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }
    for (int i = 0; i < a->capacidade_max; i++)
    {
        a->slots[i].p = NULL;
        a->slots[i].tempo_restante = 0;
    }
}

atividade *criar_atividade(etapa *dona, int id, char *nome, int tempo, float failrate, int qtd_uf)
{
    if (id < 0 || tempo <= 0 || failrate < 0.0 || failrate > 1.0 || qtd_uf <= 0)
    {
        printf("Erro: dados inválidos na ATIVIDADE %d.\n", id);
        exit(1);
    }

    atividade *nova = malloc(sizeof(atividade));
    if (!nova)
    {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }
    nova->id = id;
    strcpy(nova->nome, nome);
    nova->etapa_dona = dona;
    nova->qtd_uf = qtd_uf;
    nova->capacidade_max = qtd_uf;
    nova->ocupacao = 0;
    nova->tempo_de_processamento = tempo;
    nova->failrate = failrate;
    nova->f = NULL;
    nova->proxima_atividade = NULL;
    criar_slots(nova);

    if (dona->primeira_atividade == NULL)
        dona->primeira_atividade = nova;
    else
        dona->ultima_atividade->proxima_atividade = nova;
    dona->ultima_atividade = nova;

    return nova;
}

etapa *criar_etapa(simulacao *s, int id, char *nome, int capacidade, int qtdAtividades)
{
    if (id < 0 || capacidade < 0 || qtdAtividades <= 0)
    {
        printf("Erro: dados inválidos na ETAPA %d.\n", id);
        exit(1);
    }

    etapa *nova = malloc(sizeof(etapa));
    if (!nova)
    {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }
    nova->id = id;
    strcpy(nova->nome, nome);
    nova->num_atividades = qtdAtividades;
    nova->primeira_atividade = NULL;
    nova->ultima_atividade = NULL;
    nova->proxima_etapa = NULL;
    nova->ocupacao = 0;
    nova->capacidade_max = capacidade;
    nova->qtd_produtos_concluidos = 0;
    nova->qtd_produtos_entraram = 0;
    nova->falhas = 0;

    if (s->linha == NULL)
    {
        s->linha = malloc(sizeof(etapas));
        s->linha->primeira_etapa = nova;
        nova->etapa_anterior = NULL;
    }
    else
    {
        s->linha->ultima_etapa->proxima_etapa = nova;
        nova->etapa_anterior = s->linha->ultima_etapa;
    }
    s->linha->ultima_etapa = nova;

    nova->f = malloc(sizeof(fila));
    nova->f->inicio = NULL;
    nova->f->fim = NULL;
    nova->f->em_fila = 0;

    return nova;
}

// Função principal de leitura do arquivo
void lerEntrada(simulacao *s, FILE *arquivo)
{
    char linha[200];

    // Primeiro inicializa a simulação com valores padrão
    inicializar_simulacao(s);

    // Ignora primeira linha
    if (!fgets(linha, sizeof(linha), arquivo))
    {
        printf("Erro: arquivo vazio.\n");
        exit(1);
    }

    // SIMULACAO
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "SIMULACAO %99s %d %d", s->nome_cenario, &s->semente, &s->max_ticks) != 3)
    {
        printf("Erro: linha SIMULACAO inválida.\n");
        exit(1);
    }
    if (s->semente < 0 || s->max_ticks <= 0)
    {
        printf("Erro: valores inválidos em SIMULACAO.\n");
        exit(1);
    }

    // PRODUTOS
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "PRODUTOS %d %d %99s", &s->n_produtos_total, &s->vazao, s->modelo_produto) != 3)
    {
        printf("Erro: linha PRODUTOS inválida.\n");
        exit(1);
    }
    if (s->n_produtos_total <= 0 || s->vazao <= 0)
    {
        printf("Erro: valores inválidos em PRODUTOS.\n");
        exit(1);
    }

    // LINHA_PRODUCAO
    int qtdEtapas;
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "LINHA_PRODUCAO %d", &qtdEtapas) != 1 || qtdEtapas <= 0)
    {
        printf("Erro: linha LINHA_PRODUCAO inválida.\n");
        exit(1);
    }

    for (int i = 0; i < qtdEtapas; i++)
    {
        int id, qtdAtividades, capacidade;
        float taxaFalha;
        char nomeEtapa[50];

        if (!fgets(linha, sizeof(linha), arquivo) ||
            sscanf(linha, "ETAPA %d %d %d %f %49s", &id, &qtdAtividades, &capacidade, &taxaFalha, nomeEtapa) != 5)
        {
            printf("Erro: linha ETAPA inválida.\n");
            exit(1);
        }
        if (id < 0 || capacidade < 0 || taxaFalha < 0.0 || taxaFalha > 1.0 || qtdAtividades <= 0)
        {
            printf("Erro: dados inválidos na ETAPA %d.\n", id);
            exit(1);
        }

        etapa *e = criar_etapa(s, id, nomeEtapa, capacidade, qtdAtividades);

        for (int j = 0; j < qtdAtividades; j++)
        {
            int idA, tempo, qtdUF;
            float taxaFalhaA;
            char nomeAtividade[50];

            if (!fgets(linha, sizeof(linha), arquivo) ||
                sscanf(linha, "ATIVIDADE %d %d %f %d %49s", &idA, &tempo, &taxaFalhaA, &qtdUF, nomeAtividade) != 5)
            {
                printf("Erro: linha ATIVIDADE inválida.\n");
                exit(1);
            }

            criar_atividade(e, idA, nomeAtividade, tempo, taxaFalhaA, qtdUF);
        }
    }
}
