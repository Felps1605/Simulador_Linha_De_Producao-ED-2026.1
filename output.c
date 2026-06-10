
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "output.h"

/*
    output.c

    Funções responsáveis por gerar relatórios e calcular métricas
    a partir dos dados coletados durante a simulação.

    Ideia principal:
    - O relatório por etapa usa os produtos da pilha de concluídos.
    - Cada produto concluído possui um histórico de eventos de etapa.
    - Cada evento de etapa possui um histórico de eventos de atividade.
*/

/* ============================================================
   PROTÓTIPOS PRIVADOS
   ============================================================ */

static int etapa_valida(etapa *e);
static int pilha_valida(pilha *p);

static int buscar_tempo_na_etapa(produto *p, int id_etapa);
static int buscar_tempo_em_fila_nas_atividades(evento_etapa *ev_etapa);
static tempos buscar_tempos_de_fila_na_etapa(produto *p, int id_etapa);

static float tempo_medio_na_etapa(etapa *e, pilha *produtos);
static int tempo_maximo_na_etapa(etapa *e, pilha *produtos);
static tempos tempo_medio_em_fila_por_etapa(etapa *e, pilha *produtos);

static int tempo_minimo_na_etapa(etapa *e);

static float tempo_medio_na_linha(simulacao *s);
static float tempo_medio_em_espera(simulacao *s);

static float tempo_medio_fila_atividade(atividade *a, pilha *produtos);


/* ============================================================
   FUNÇÕES DE RELATÓRIO
   ============================================================ */

void mostrar_metadados(simulacao *s)
{
    if (!s)
    {
        printf("Simulacao invalida.\n");
        return;
    }

    int meta_batida = (s->produtos_concluidos >= s->meta);

    printf("\n=== METADADOS ===\n");

    printf("Id da simulacao: %s\n", s->id_simulacao);
    printf("Semente utilizada: %d\n", s->semente);
    printf("Arquivo de entrada: %s\n", s->arquivo_entrada);
    printf("Nome do cenario: %s\n", s->nome_cenario);
    printf("Tick_fim: %d\n", s->tick_atual);
    printf("Produtos_concluidos: %d\n", s->produtos_concluidos);
    printf("Produtos criados: %d\n", s->produtos_criados);
    printf("Tempo_medio_linha: %.2f\n", tempo_medio_na_linha(s));
    printf("Tempo_medio_espera: %.2f\n", tempo_medio_em_espera(s));
    printf("Falhas_totais: %d\n", s->falhas_totais);
    printf("Meta alcancada(%.1f): %s\n",s->meta, meta_batida ? "Sim" : "Nao");

    if (!meta_batida)
    {
        printf("Produtos faltantes: %.0f\n",
               s->meta - s->produtos_concluidos);
    }
}

void mostrar_relatorio_etapas(simulacao *s)
{
    if (!s || !s->linha)
    {
        printf("Linha invalida!\n");
        return;
    }

    if (!s->linha->primeira_etapa)
    {
        printf("Nenhuma etapa registrada na linha.\n");
        return;
    }

    printf("\n-------------- RELATORIO DE ETAPAS --------------\n");

    etapa *atual = s->linha->primeira_etapa;

    while (atual)
    {
        tempos tf = tempo_medio_em_fila_por_etapa(atual, s->concluidos);

        printf("\nETAPA %d, %s:\n", atual->id, atual->nome);

        printf("\tAtividades: %d\n", atual->num_atividades);
        printf("\tFalhas totais: %d\n", atual->falhas);
        printf("\tQuantidade de produtos que entraram: %d\n", atual->qtd_produtos_entraram);
        printf("\tQuantidade de produtos concluidos: %d\n", atual->qtd_produtos_concluidos);

        if (atual->qtd_produtos_entraram > 0)
        {
            printf("\tMedia de falhas por produto: %.2f\n", (float)atual->falhas / atual->qtd_produtos_entraram);
        }
        else
        {
            printf("\tMedia de falhas por produto: 0.00\n");
        }

        printf("\tTempo minimo: %d\n", tempo_minimo_na_etapa(atual));

        printf("\tTempo medio: %.2f\n", tempo_medio_na_etapa(atual, s->concluidos));

        printf("\tMaior tempo: %d\n", tempo_maximo_na_etapa(atual, s->concluidos));

        printf("\tTempo medio total em filas de atividade: %.2f\n", tf.tempo_em_fila_de_atividades);

        printf("\tTempo medio na fila de prontos da etapa: %.2f\n", tf.tempo_em_fila_de_prontos);

        atual = atual->proxima_etapa;
    }
}

void mostrar_relatorio_atividades(simulacao *s)
{
    if (!s || !s->linha)
    {
        printf("Linha invalida!\n");
        return;
    }

    if (!s->linha->primeira_etapa)
    {
        printf("Nenhuma etapa registrada na linha.\n");
        return;
    }

    printf("\n-------------- RELATORIO DE ATIVIDADES --------------\n");

    etapa *e_atual = s->linha->primeira_etapa;

    while (e_atual)
    {
        printf("\nETAPA %d, %s:\n", e_atual->id, e_atual->nome);
        atividade * a_atual = e_atual->primeira_atividade;
        while(a_atual)
        {
            printf("\n  ATIVIDADE %d, %s:\n", a_atual->id, a_atual->nome);
            printf("\tCapacidade: %d \n", a_atual->capacidade_max);//possivelmente depois mostrar capacidade por uf e qtd de ufs
            printf("\tTempo de execucao: %d \n", a_atual->tempo_de_processamento);
            float tmf = tempo_medio_fila_atividade(a_atual, s->concluidos);
            printf("\tTempo medio em fila: %.2f\n", tmf );
            printf("\tTempo medio total:  %.2f\n", tmf + a_atual->tempo_de_processamento);

            a_atual = a_atual->proxima_atividade;
        }
        e_atual = e_atual->proxima_etapa;
    }
}

//void mostrar_trajetoria_produtos(){}

/* ============================================================
   FUNÇÕES DE CÁLCULO GERAL
   ============================================================ */

static float tempo_medio_na_linha(simulacao *s)
{
    if (!s || !pilha_valida(s->concluidos))
    {
        return 0;
    }

    float soma = 0;
    int quantidade = 0;

    produto *atual = s->concluidos->topo;

    while (atual)
    {
        if (atual->tick_saida_linha >= 0 &&
            atual->tick_criacao >= 0)
        {
            soma += atual->tick_saida_linha - atual->tick_criacao;
            quantidade++;
        }

        atual = atual->proximo_produto;
    }

    if (quantidade == 0)
    {
        return 0;
    }

    return soma / quantidade;
}

static float tempo_medio_em_espera(simulacao *s)
{
    if (!s || !s->linha || !pilha_valida(s->concluidos))
    {
        return 0;
    }

    float media_fila_entrada = 0;
    int quantidade = 0;

    produto *p = s->concluidos->topo;

    while (p)
    {
        if (p->tick_criacao >= 0 &&
            p->tick_entrada_linha >= 0)
        {
            media_fila_entrada += p->tick_entrada_linha - p->tick_criacao;
            quantidade++;
        }

        p = p->proximo_produto;
    }

    if (quantidade > 0)
    {
        media_fila_entrada /= quantidade;
    }

    float soma_filas_etapas = 0;

    etapa *e = s->linha->primeira_etapa;

    while (e)
    {
        tempos tf = tempo_medio_em_fila_por_etapa(e, s->concluidos);

        soma_filas_etapas += tf.tempo_em_fila_de_atividades;
        soma_filas_etapas += tf.tempo_em_fila_de_prontos;

        e = e->proxima_etapa;
    }

    return media_fila_entrada + soma_filas_etapas;
}


/* ============================================================
   FUNÇÕES DE CÁLCULO POR ETAPA
   ============================================================ */

static int tempo_minimo_na_etapa(etapa *e)
{
    if (!etapa_valida(e))
    {
        return 0;
    }

    int soma = 0;

    atividade *atual = e->primeira_atividade;

    while (atual)
    {
        soma += atual->tempo_de_processamento;
        atual = atual->proxima_atividade;
    }

    return soma;
}

static float tempo_medio_na_etapa(etapa *e, pilha *produtos)
{
    if (!etapa_valida(e) || !pilha_valida(produtos))
    {
        return 0;
    }

    float soma = 0;
    int quantidade = 0;

    produto *p = produtos->topo;

    while (p)
    {
        int tempo = buscar_tempo_na_etapa(p, e->id);

        if (tempo >= 0)
        {
            soma += tempo;
            quantidade++;
        }

        p = p->proximo_produto;
    }

    if (quantidade == 0)
    {
        return 0;
    }

    return soma / quantidade;
}

static int tempo_maximo_na_etapa(etapa *e, pilha *produtos)
{
    if (!etapa_valida(e) || !pilha_valida(produtos))
    {
        return 0;
    }

    int maior = 0;

    produto *p = produtos->topo;

    while (p)
    {
        int tempo = buscar_tempo_na_etapa(p, e->id);

        if (tempo > maior)
        {
            maior = tempo;
        }

        p = p->proximo_produto;
    }

    return maior;
}

static tempos tempo_medio_em_fila_por_etapa(etapa *e, pilha *produtos)
{
    tempos media;
    media.tempo_em_fila_de_atividades = 0;
    media.tempo_em_fila_de_prontos = 0;

    if (!etapa_valida(e) || !pilha_valida(produtos))
    {
        return media;
    }

    float soma_fila_atividades = 0;
    float soma_fila_prontos = 0;
    int quantidade = 0;

    produto *p = produtos->topo;

    while (p)
    {
        tempos t = buscar_tempos_de_fila_na_etapa(p, e->id);

        soma_fila_atividades += t.tempo_em_fila_de_atividades;
        soma_fila_prontos += t.tempo_em_fila_de_prontos;

        quantidade++;

        p = p->proximo_produto;
    }

    if (quantidade == 0)
    {
        return media;
    }

    media.tempo_em_fila_de_atividades = soma_fila_atividades / quantidade;
    media.tempo_em_fila_de_prontos = soma_fila_prontos / quantidade;

    return media;
}


/* ============================================================
   FUNÇÕES DE CÁLCULO POR ATIVIDADE
   ============================================================ */
   
float tempo_medio_fila_atividade(atividade *a, pilha *produtos)
{
    if (!a || !produtos || produtos->em_pilha == 0)
    {
        return 0;
    }

    float soma_tempos = 0;
    int produtos_contados = 0;

    produto *p = produtos->topo;

    while (p)
    {
        int tempo_produto = 0;
        int passou_na_atividade = 0;

        evento_etapa *ev_etapa = p->historico_etapas;

        while (ev_etapa)
        {
            evento_atividade *ev_atividade = ev_etapa->historico_atividades;

            while (ev_atividade)
            {
                if (ev_atividade->a == a)
                {
                    if (ev_atividade->tick_fila >= 0 && ev_atividade->tick_inicio_processamento >= 0)
                    {
                        tempo_produto += ev_atividade->tick_inicio_processamento - ev_atividade->tick_fila;

                        passou_na_atividade = 1;
                    }
                }

                ev_atividade = ev_atividade->proximo_evento;
            }

            ev_etapa = ev_etapa->proximo_evento;
        }

        if (passou_na_atividade)
        {
            soma_tempos += tempo_produto;
            produtos_contados++;
        }

        p = p->proximo_produto;
    }

    if (produtos_contados == 0)
    {
        return 0;
    }

    return soma_tempos / produtos_contados;
}

/* ============================================================
   FUNÇÕES QUE EXTRAEM INFORMAÇÕES DOS EVENTOS
   ============================================================ */

static int buscar_tempo_na_etapa(produto *p, int id_etapa)
{
    if (!p || !p->historico_etapas)
    {
        return -1;
    }

    int soma = 0;
    int achou = 0;

    evento_etapa *ev = p->historico_etapas;

    while (ev)
    {
        if (ev->e && ev->e->id == id_etapa)
        {
            if (ev->tick_inicio >= 0 &&
                ev->tick_fim >= 0)
            {
                soma += ev->tick_fim - ev->tick_inicio;
                achou = 1;
            }
        }

        ev = ev->proximo_evento;
    }

    if (!achou)
    {
        return -1;
    }

    return soma;
}

static tempos buscar_tempos_de_fila_na_etapa(produto *p, int id_etapa)
{
    tempos resultado;
    resultado.tempo_em_fila_de_atividades = 0;
    resultado.tempo_em_fila_de_prontos = 0;

    if (!p || !p->historico_etapas)
    {
        return resultado;
    }

    int tentativas = 0;

    evento_etapa *ev = p->historico_etapas;

    while (ev)
    {
        if (ev->e && ev->e->id == id_etapa)
        {
            resultado.tempo_em_fila_de_atividades += buscar_tempo_em_fila_nas_atividades(ev);

            if (ev->tick_conclusao >= 0 && ev->tick_fim >= 0)
            {
                resultado.tempo_em_fila_de_prontos += ev->tick_fim - ev->tick_conclusao;
            }

            tentativas++;
        }

        ev = ev->proximo_evento;
    }

    /*
        Aqui existem duas opções:

        1. Dividir por tentativas:
           mede o tempo médio de fila por tentativa.

        2. Não dividir:
           mede o tempo total de fila do produto naquela etapa.

        Para relatório "tempo médio que um produto passou em fila na etapa",
        eu recomendo NÃO dividir por tentativas, porque retrabalho também
        consumiu tempo real do produto.

        Por isso deixei sem divisão.
    */

    return resultado;
}

static int buscar_tempo_em_fila_nas_atividades(evento_etapa *ev_etapa)
{
    if (!ev_etapa)
    {
        return 0;
    }

    int soma = 0;

    evento_atividade *ev = ev_etapa->historico_atividades;

    while (ev)
    {
        if (ev->tick_fila >= 0 &&  ev->tick_inicio_processamento >= 0)
        {
            soma += ev->tick_inicio_processamento - ev->tick_fila;
        }

        ev = ev->proximo_evento;
    }

    return soma;
}

/* ============================================================
   FUNÇÕES AUXILIARES
   ============================================================ */

static int etapa_valida(etapa *e)
{
    return (e != NULL);
}

static int pilha_valida(pilha *p)
{
    return p != NULL && p->em_pilha > 0 && p->topo != NULL;
}