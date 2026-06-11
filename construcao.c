#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "construcao.h"


//funções de construção
#include <time.h>
#include <stdio.h>

void gerar_id_simulacao(simulacao *s)
{
    time_t agora;
    struct tm *info_tempo;

    time(&agora);
    info_tempo = localtime(&agora);

    strftime(s->id_simulacao, sizeof(s->id_simulacao),"%Y%m%d_%H%M%S", info_tempo);
}

void inicializar_simulacao(simulacao *s)
{   
    gerar_id_simulacao(s);//s->id_simulacao = horario;
    s->fila_entrada = NULL;
    s->lixo = NULL;
    s->concluidos = NULL;
    s->linha = NULL;
    s->produto_id = 1;
    s->etapa_id = 1;
    s->produtos_criados = 0;
    s->produtos_concluidos = 0;
    s->MODO_MANUAL = 0; // inicia em modo manual, pode ser alterado depois
    s->n_etapas = 2;

    strcpy(s->arquivo_entrada, "arquivocaijcna");
    strcpy(s->nome_cenario, "nome_aleatorio");

    s->vazao = 2;
    s->n_produtos_total = 20; //parte arbitrária por enquanto
    s->meta = (float)s->n_produtos_total *0.5; //a meta é concluir com sucesso 50 % dos produtos criados
    s->max_ticks = 100000;
    s->semente = 1; //(int)time(NULL); // semente aleatoria baseada no tempo atual, se houver entrada de semente no arquivo deve ser substituida
}
void criar_slots(atividade *a)
{
    int capacidade = a->capacidade_max;
    a->slots = malloc(sizeof(slot) * capacidade);
    if (!a->slots)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    for (int i = 0; i < capacidade; i++)
    {
        a->slots[i].p = NULL;
        a->slots[i].tempo_restante = 0;
    }
}
void criar_atividade(etapa *dona, int indice, int capacidade)
{
    atividade *nova = malloc(sizeof(atividade));
    if (!nova)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    nova->id = dona->id * 100 + indice;
    snprintf(nova->nome, sizeof(nova->nome), "Atividade %d", nova->id);
    nova->etapa_dona = dona;
    nova->qtd_uf = 1; // quantidade de unidades funcionais, arbitraria, pode ser alterada depois
    nova->capacidade_max = capacidade * nova->qtd_uf; // capacidade arbitraria, pode ser alterada depois
    nova->ocupacao = 0;
    nova->tempo_de_processamento = 5;
    nova->failrate = 0.1; // taxa de falha arbitraria, pode ser alterada depois
    nova->f = NULL;
    nova->proxima_atividade = NULL;
    criar_slots(nova);
    printf("Atividade %d criada, setando os ponteiros\n", nova->id);
    if ((dona->primeira_atividade) == NULL)
    {
        dona->primeira_atividade = nova;
    }
    else
    {
        dona->ultima_atividade->proxima_atividade = nova;
    }
    dona->ultima_atividade = nova;
    printf("Ponteiros setados\n");
}
void criar_etapa(simulacao *s)
{
    etapa *nova = malloc(sizeof(etapa));
    if (!nova)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    nova->id = s->etapa_id;
    snprintf(nova->nome, sizeof(nova->nome), "Etapa %d", s->etapa_id); // default, pode ser alterado depois
    s->etapa_id++;
    nova->primeira_atividade = NULL;
    nova->ultima_atividade = NULL;
    nova->proxima_etapa = NULL;
    nova->ocupacao = 0;
    nova->capacidade_max = 0;
    nova->qtd_produtos_concluidos = 0;
    nova->qtd_produtos_entraram =0;
    nova->falhas = 0;

    if (s->linha == NULL)
    {
        s->linha = malloc(sizeof(etapas));
        if (!s->linha)
        {
            printf("\n\nErro ao alocar memória! \n\n");
            exit(1);
        }
        s->linha->primeira_etapa = nova;
        nova->etapa_anterior = NULL;
    }
    else
    {
        s->linha->ultima_etapa->proxima_etapa = nova;
        nova->etapa_anterior = s->linha->ultima_etapa;
    }
    s->linha->ultima_etapa = nova;

    // fila de prontos para ir para a próxima etapa
    nova->f = malloc(sizeof(fila));
    if (!nova->f)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    nova->f->inicio = NULL;
    nova->f->fim = NULL;
    nova->f->em_fila = 0;

    printf("Etapa %d criada, criando atividades\n", nova->id);
    nova->num_atividades = 2; // arbitrario
    for (int i = 1; i <= nova->num_atividades; i++)
    {
        int catividade = 2;
        criar_atividade(nova, i, catividade);
        nova->capacidade_max = nova->capacidade_max + catividade;
    }
    int capacidade_filas = 10;
    nova->capacidade_max = nova->capacidade_max + capacidade_filas;
    printf("Etapa %d e suas atividades concluidas\n", nova->id);
}


