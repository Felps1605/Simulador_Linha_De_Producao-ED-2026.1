
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "output.h"
#include "execucao.h"

/*
    output.c

    Funções responsáveis por gerar relatórios e calcular métricas
    a partir dos dados coletados durante a simulação.

*/

/* ============================================================
   PROTÓTIPOS PRIVADOS
   ============================================================ */

static void trajetoria(produto * p);
static int tempo_filas_atividades(produto * p);
static int em_linha(simulacao *s);
static void mostrar_arvore(simulacao *s , noa *raiz);

static void imprimir_metadados_arquivo(FILE *arquivo, simulacao *s);
static void imprimir_relatorio_etapas_arquivo(FILE *arquivo, simulacao *s);
static void imprimir_relatorio_atividades_arquivo(FILE *arquivo, simulacao *s);
static void imprimir_relatorio_produtos_arquivo(FILE *arquivo, simulacao *s);
static void imprimir_historico_produto_arquivo(FILE *arquivo, simulacao *s, produto *p);
static void trajetoria_arquivo(FILE *arquivo, produto *p);
static void mostrar_arvore_arquivo(FILE *arquivo, simulacao *s , noa *raiz);

/* ============================================================
   FUNÇOES PUBLICAS
   ============================================================ */
void relatorio_simulacao(simulacao *s)//cria o relatorio completo em forma de arquivo
{
    if (!s)
    {
        printf("Simulacao invalida.\n");
        return;
    }

    //inicializar_resumos(s);

    //preencher_resumos(s->resumo, s->concluidos);

    char nome_arquivo[150];

    snprintf(nome_arquivo,
             sizeof(nome_arquivo),
             "relatorio_%s.txt",
             s->id_simulacao);

    FILE *arquivo = fopen(nome_arquivo, "w");

    if (!arquivo)
    {
        printf("Erro ao criar arquivo de relatorio.\n");
        liberar_resumos(s);
        return;
    }

    imprimir_metadados_arquivo(arquivo, s);
    imprimir_relatorio_etapas_arquivo(arquivo, s);
    imprimir_relatorio_atividades_arquivo(arquivo, s);
    imprimir_relatorio_produtos_arquivo(arquivo, s);

    fclose(arquivo);

    printf("Relatorio salvo em: %s\n", nome_arquivo);

}

void imprimir_metadados(simulacao *s)
{
    if (!s)
    {
        printf("Simulacao invalida.\n");
        return;
    }

    printf("\n=== METADADOS ===\n");

    printf("Id da simulacao: %s\n", s->id_simulacao);

    printf("Semente utilizada: %d\n", s->semente);

    printf("Arquivo de entrada: %s\n", s->arquivo_entrada);

    printf("Nome do cenario: %s\n", s->nome_cenario);

    printf("Produto: %s\n", s->modelo_produto);

    printf("Tick fim: %d\n", s->tick_atual);

    printf("Produtos concluidos: %d\n", s->produtos_concluidos);

    printf("Produtos criados: %d\n", s->produtos_criados);
    
    printf("Falhas_totais: %d\n", s->falhas_totais);
    
    if(s->tempo_excedido){
        printf("Tempo limite atingido\n");
        printf("Produtos em linha: %d \n", em_linha(s) );
    }

    if(!s->resumo){
        //printf("Nao foi possivel calcular as metricas restantes\n");
        return;
    }

    printf("Tempo medio na linha: %.2f\n", s->resumo->tempo_medio_total);

    printf("  Tempo minimo na linha: %d\n", s->resumo->tempo_minimo);

    printf("  Tempo medio em espera: %.2f\n", s->resumo->tempo_medio_em_espera);

    printf("    Tempo medio na fila de entrada: %.2f\n", s->resumo->tempo_medio_na_fila_entrada);

    printf("    Tempo medio total nas filas da linha: %.2f\n", s->resumo->tempo_medio_filas_atividades);

    printf("  Tempo medio em retrabalho: %.2f\n", s->resumo->tempo_medio_retrabalho);

    

    printf("Meta alcancada(%.1f): %s\n", s->meta, s->resumo->meta_alcancada ? "SIM" : "NAO");

    if (!s->resumo->meta_alcancada)
        printf("Produtos faltantes: %.0f\n", s->meta - s->produtos_concluidos);
    

}

void imprimir_relatorio_etapas(simulacao *s)
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

        printf("\nETAPA %d, %s:\n", atual->id, atual->nome);

        printf("\tAtividades: %d\n", atual->num_atividades);

        printf("\tCapacidade total: %d\n", atual->capacidade_max);

        printf("\tFalhas totais: %d\n", atual->falhas);

        printf("\tFailrate: %.2f\n", atual->failrate);

        printf("\tQuantidade de produtos que entraram: %d\n", atual->qtd_produtos_entraram);

        printf("\tQuantidade de produtos concluidos: %d\n", atual->qtd_produtos_concluidos);

        if(!s->resumo || !atual->resumo){
            //printf("Nao foi possivel calcular as metricas restantes\n");
            atual = atual->proxima_etapa;
            continue;
        }

        printf("\tMedia de falhas por produto: %.2f\n", atual->resumo->falhas_por_produto);

        printf("\tTempo medio: %.2f\n", atual->resumo->tempo_medio);

        printf("\t  Tempo minimo: %d\n", atual->resumo->tempo_minimo);

        printf("\t  Tempo medio total em filas de atividade: %.2f\n", atual->resumo->tempo_medio_filas_atividades);

        printf("\t  Tempo medio na fila de prontos da etapa: %.2f\n", atual->resumo->tempo_medio_fila_prontos);

        printf("\tMaior tempo: %d\n", atual->resumo->maior_tempo);

        atual = atual->proxima_etapa;
    }
}

void imprimir_relatorio_atividades(simulacao *s)
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

        atividade *a_atual = e_atual->primeira_atividade;
        while (a_atual)
        {
            printf("\n  ATIVIDADE %d, %s:\n", a_atual->id, a_atual->nome);

            printf("\tCapacidade: %d \n", a_atual->capacidade_max);

            printf("\t  Capacidade por Unidade Funcional: %d\n", a_atual->capacidade_max / a_atual->qtd_uf);

            printf("\t  Quantidade de Unidades Funcionais: %d\n", a_atual->qtd_uf);

            printf("\tFailrate: %.2f\n", a_atual->failrate);

            if(!s->resumo || !a_atual->resumo){
            //printf("Nao foi possivel calcular as metricas restantes\n");
            a_atual = a_atual->proxima_atividade;
            continue;
        }

            printf("\tTempo medio total:  %.2f\n", a_atual->resumo->tempo_medio_total);

            printf("\t  Tempo de execucao: %d \n", a_atual->tempo_de_processamento);

            printf("\t  Tempo medio em fila: %.2f\n", a_atual->resumo->tempo_medio_na_fila);

            printf("\tMaior Tempo: %d \n ", a_atual->resumo->maior_tempo);

            a_atual = a_atual->proxima_atividade;
        }
        e_atual = e_atual->proxima_etapa;
    }
}

void imprimir_relatorio_produtos(simulacao *s)
{   
    if(!s){
        printf("Simulacao invalida\n");
        return;
    }
        
    printf("\n-------RELATORIO DE PRODUTOS--------\n");
    mostrar_arvore(s , s->finalizados);
    printf("\nRELATORIO DE PRODUTOS EM LINHA\n");
    mostrar_arvore(s, s->em_linha);
}

void imprimir_historico_produto(simulacao *s, produto *p)
{   
    if(!s){
        printf("Simulacao invalida\n");
        return;
    }
    if (!p)
    {
        printf("Produto nao encontrado\n");
        return;
    }
    printf("\n\n---Produto %d---\n\n", p->id);
    printf("Modelo: %s\n", s->modelo_produto);
    if (p->tick_saida_linha > 0)
    {
        printf("Tick de Criacao: %d \n", p->tick_criacao);
        printf("Tick de Entrada na linha: %d \n", p->tick_entrada_linha);
        printf("Tick de Saida da linha: %d \n", p->tick_saida_linha);
        printf("Localizacao atual: Pilha de Concluidos \n");
        int tempo_filas_atividade = tempo_filas_atividades(p);
        printf("Tempo Total no Sistema: %d \n", p->tick_saida_linha - p->tick_criacao);
        printf("  Tempo na fila de entrada: %d \n", p->tick_entrada_linha - p->tick_criacao);
        printf("  Tempo na linha: %d \n", p->tick_saida_linha - p->tick_entrada_linha);
        printf("    Filas de atividade: %d \n", tempo_filas_atividade);
        printf("    Processamento e Retrabalho : %d \n",(p->tick_saida_linha - p->tick_entrada_linha) - tempo_filas_atividade);
        printf("Tempo Total em espera: %d \n", tempo_filas_atividade + (p->tick_entrada_linha - p->tick_criacao));
    }
    else if (p->tick_saida_linha < 0)
    {
        printf("Tick de Criacao: %d \n", p->tick_criacao);
        printf("Tick de Entrada na linha: %d \n", p->tick_entrada_linha);
        printf("Localizacao atual: Pilha de Lixo \n");
    }
    else if(p->tick_entrada_linha > 0)
    {
        printf("Tick de Criacao: %d \n", p->tick_criacao);
        printf("Tick de Entrada na linha: %d \n", p->tick_entrada_linha);
        printf("Localizacao atual: EM LINHA \n");
    }else {
        printf("Tick de Criacao: %d \n", p->tick_criacao);
        printf("Localizacao atual: Fila de entrada da linha \n");
        return;
    }
    
    
    
    
    printf("Falhas: %d \n", p->falhas);

    trajetoria(p);
}

void preencher_resumos(resumo_simulacao *rs, pilha *produtos)
{ // percorre cada evento de atividade de cada evento etapa e de cada produto e preenche os resumos
    if(!rs)
    {
        //printf("Sem relatorios a preencher\n");
        return;
    }
    printf("Iniciando preenchimento de relatorios \n");
    produto *p = produtos->topo;
    float soma_tempo_medio_total = 0;
    float soma_tempo_fila_entrada = 0;
    while (p)
    {

        soma_tempo_medio_total += (p->tick_saida_linha - p->tick_criacao);
        soma_tempo_fila_entrada += (p->tick_entrada_linha - p->tick_criacao);

        evento_etapa *ev_et = p->historico_etapas;
        while (ev_et)
        {

            int etapa_valida = !ev_et->falhou && ev_et->tick_inicio >= 0 && ev_et->tick_fim >= 0;

            evento_atividade *ev_at = ev_et->historico_atividades;

            while (ev_at)
            {
                ev_at->a->resumo->passagens++;
                int tempo_total_at = ev_at->tick_fim_processamento - ev_at->tick_fila;
                int tempo_fila = ev_at->tick_inicio_processamento - ev_at->tick_fila;

                
                ev_at->a->resumo->soma_tempo_na_fila += tempo_fila;

                ev_at->a->resumo->soma_tempo_total += tempo_total_at;

                if (tempo_total_at > ev_at->a->resumo->maior_tempo)
                    ev_at->a->resumo->maior_tempo = tempo_total_at;

                if (etapa_valida)
                    ev_at->a->resumo->soma_tempo_na_fila_etapa_valida += tempo_fila;

                ev_at = ev_at->proximo_evento;
            }
            if (etapa_valida) // exclui da contagem as tentativas falhas (bagunçam o relatorio)
            {

                ev_et->e->resumo->passagens++;

                int tempo_total_et = ev_et->tick_fim - ev_et->tick_inicio;

                ev_et->e->resumo->soma_tempo_total += tempo_total_et;

                ev_et->e->resumo->soma_tempo_fila_prontos += ev_et->tick_fim - ev_et->tick_conclusao;

                if (tempo_total_et > ev_et->e->resumo->maior_tempo)
                    ev_et->e->resumo->maior_tempo = tempo_total_et;
            }

            ev_et = ev_et->proximo_evento;
        }
        p = p->proximo_produto;
    }

    // dados coletados, agora é preciso ir em cada resumo_etapa e resumo_atividade e setar os dados

    float soma_tempo_minimo_total = 0;
    float soma_tempo_filas_total = 0;
    for (int i = 0; i < rs->s->n_etapas; i++)
    {

        resumo_etapa *re = &(rs->resumos_etapas[i]);
        float soma_tempo_minimo_etapa = 0;
        ;
        float soma_tempo_filas_etapa = 0;
        for (int j = 0; j < re->e->num_atividades; j++)
        {
            resumo_atividade *ra = &(re->resumos_atividades[j]);
            if (ra->passagens && re->passagens)
            {
                ra->tempo_medio_na_fila = ra->soma_tempo_na_fila / ra->passagens;
                ra->tempo_medio_total = ra->soma_tempo_total / ra->passagens;
                ra->tempo_medio_na_fila_etapas_validas = ra->soma_tempo_na_fila_etapa_valida / re->passagens;
                // ra->maior_tempo ja ta setado
                // ra->passagens também já setado
            }
            else
            {
                ra->tempo_medio_na_fila = 0;
                ra->tempo_medio_total = 0;
                ra->maior_tempo = 0;
                ra->tempo_medio_na_fila_etapas_validas = 0;
            }

            soma_tempo_minimo_etapa += ra->a->tempo_de_processamento;
            /*não posso calcular o tempo medio em filas de atividade da etapa só somando tempo medio de cada atividade,
             pois essas incluem execuções de atividades em eventos etapas inválidos*/

            soma_tempo_filas_etapa += ra->tempo_medio_na_fila_etapas_validas;
        }
        if (re->e->resumo->passagens && re->e->qtd_produtos_entraram)
        {
            re->falhas_por_produto = (float)re->e->falhas / re->e->qtd_produtos_entraram;
            re->tempo_medio = re->soma_tempo_total / re->e->resumo->passagens;
            re->tempo_medio_fila_prontos = re->soma_tempo_fila_prontos / re->e->resumo->passagens;
        }
        else
        {
            re->falhas_por_produto = 0;
            re->tempo_medio = 0;
            re->tempo_medio_fila_prontos = 0;
        }
        re->tempo_medio_filas_atividades = soma_tempo_filas_etapa;
        // re->maior_tempo ja ta setado
        re->tempo_minimo = soma_tempo_minimo_etapa;

        re->tempo_medio_filas_total = re->tempo_medio_filas_atividades + re->tempo_medio_fila_prontos;

        soma_tempo_minimo_total += re->tempo_minimo;
        soma_tempo_filas_total += re->tempo_medio_filas_total;
    }
    rs->tempo_medio_total = soma_tempo_medio_total / produtos->em_pilha;

    rs->tempo_minimo = soma_tempo_minimo_total;

    rs->tempo_medio_na_fila_entrada = soma_tempo_fila_entrada / produtos->em_pilha;
    rs->tempo_medio_filas_atividades = soma_tempo_filas_total;

    rs->tempo_medio_em_espera = rs->tempo_medio_filas_atividades + rs->tempo_medio_na_fila_entrada;

    rs->meta_alcancada = (rs->s->produtos_concluidos >= rs->s->meta) ? 1 : 0;

    rs->tempo_medio_retrabalho = rs->tempo_medio_total - rs->tempo_medio_em_espera - rs->tempo_minimo;

    printf("Relatorios prontos\n");
}

void inicializar_resumos(simulacao *s)
{
    printf("Inicializando relatorios \n");
    if(!s->concluidos)
    {
        printf("Relatorios simplificados inicializados\n");
        s->resumo = NULL;//so pra garantir
        return;
    }
    resumo_simulacao *rs = malloc(sizeof(resumo_simulacao));
    rs->resumos_etapas = malloc(s->n_etapas * sizeof(resumo_etapa));
    if (!rs->resumos_etapas)
    {
        printf("Erro ao alocar memoria para relatorios \n");
        exit(0);
    }
    // resumos_etapas agora é um vetor de resumo_etapa
    etapa *atual = s->linha->primeira_etapa;
    for (int i = 0; i < s->n_etapas; i++)
    {
        resumo_etapa *re = &rs->resumos_etapas[i];
        re->e = atual;
        atual->resumo = re;
        re->resumos_atividades = malloc(re->e->num_atividades * sizeof(resumo_atividade));
        if (!re->resumos_atividades)
        {
            printf("Erro ao alocar memoria para relatorios \n");
            exit(0);
        }
        // resumos_atividades agora é um vetor de resumo_atividade
        atual->resumo->soma_tempo_fila_prontos = 0;
        atual->resumo->soma_tempo_total = 0;
        atual->resumo->maior_tempo = 0;
        atual->resumo->passagens = 0;

        atividade *a_atual = atual->primeira_atividade;
        for (int j = 0; j < atual->num_atividades; j++)
        {
            re->resumos_atividades[j].a = a_atual;
            a_atual->resumo = &(re->resumos_atividades[j]);
            a_atual->resumo->soma_tempo_na_fila = 0;
            a_atual->resumo->soma_tempo_total = 0;
            a_atual->resumo->maior_tempo = 0;
            a_atual->resumo->passagens = 0;
            a_atual->resumo->soma_tempo_na_fila_etapa_valida = 0;

            a_atual = a_atual->proxima_atividade;
        }
        atual = atual->proxima_etapa;
    }
    s->resumo = rs;
    rs->s = s;
    printf("Relatorios inicializados \n");
}

void liberar_resumos(simulacao *s)
{   
    
    if(!s || !s->resumo)
        return;
    resumo_simulacao *rs = s->resumo;
    if(rs->resumos_etapas)
    {
        for (int i = 0; i < s->n_etapas; i++)
        {
            free(rs->resumos_etapas[i].resumos_atividades);
        }
        free(rs->resumos_etapas);
    }
    free(rs);
    s->resumo = NULL;
}


/* ============================================================
   FUNÇÕES DE IMPRESSAO NO ARQUIVO
   ============================================================ */

static void imprimir_metadados_arquivo(FILE *arquivo, simulacao *s)
{
    if (!arquivo)
    {
        return;
    }

    if (!s)
    {
        fprintf(arquivo, "Simulacao invalida.\n");
        return;
    }

    fprintf(arquivo, "\n=== METADADOS ===\n");

    fprintf(arquivo, "Id da simulacao: %s\n", s->id_simulacao);

    fprintf(arquivo, "Semente utilizada: %d\n", s->semente);

    fprintf(arquivo, "Arquivo de entrada: %s\n", s->arquivo_entrada);

    fprintf(arquivo, "Nome do cenario: %s\n", s->nome_cenario);

    fprintf(arquivo, "Produto: %s\n", s->modelo_produto);

    fprintf(arquivo, "Tick fim: %d\n", s->tick_atual);

    fprintf(arquivo, "Produtos concluidos: %d\n", s->produtos_concluidos);

    fprintf(arquivo, "Produtos criados: %d\n", s->produtos_criados);

    fprintf(arquivo, "Falhas_totais: %d\n", s->falhas_totais);
    
    if (s->tempo_excedido)
    {
        fprintf(arquivo, "Tempo limite atingido\n");
        fprintf(arquivo, "Produtos em linha: %d \n", em_linha(s));
    }

    if(!s->resumo)
    {
        //fprintf(arquivo, "Nao foi possivel calcular as metricas restantes.\n");
        return;
    }

    fprintf(arquivo, "Tempo medio na linha: %.2f\n", s->resumo->tempo_medio_total);

    fprintf(arquivo, "  Tempo minimo na linha: %d\n", s->resumo->tempo_minimo);

    fprintf(arquivo, "  Tempo medio em espera: %.2f\n", s->resumo->tempo_medio_em_espera);

    fprintf(arquivo, "    Tempo medio na fila de entrada: %.2f\n", s->resumo->tempo_medio_na_fila_entrada);

    fprintf(arquivo, "    Tempo medio total nas filas da linha: %.2f\n", s->resumo->tempo_medio_filas_atividades);

    fprintf(arquivo, "  Tempo medio em retrabalho: %.2f\n", s->resumo->tempo_medio_retrabalho);

    fprintf(arquivo, "Meta alcancada(%.1f): %s\n", s->meta, s->resumo->meta_alcancada ? "SIM" : "NAO");

    if (!s->resumo->meta_alcancada)
    {
        fprintf(arquivo, "Produtos faltantes: %.0f\n", s->meta - s->produtos_concluidos);
    }

}

static void imprimir_relatorio_etapas_arquivo(FILE *arquivo, simulacao *s)
{
    if (!arquivo)
    {
        return;
    }

    if (!s || !s->linha)
    {
        fprintf(arquivo, "Linha invalida!\n");
        return;
    }

    if (!s->linha->primeira_etapa)
    {
        fprintf(arquivo, "Nenhuma etapa registrada na linha.\n");
        return;
    }

    fprintf(arquivo, "\n-------------- RELATORIO DE ETAPAS --------------\n");

    etapa *atual = s->linha->primeira_etapa;

    while (atual)
    {
        fprintf(arquivo, "\nETAPA %d, %s:\n", atual->id, atual->nome);

        fprintf(arquivo, "\tAtividades: %d\n", atual->num_atividades);

        fprintf(arquivo, "\tCapacidade total: %d\n", atual->capacidade_max);

        fprintf(arquivo, "\tFalhas totais: %d\n", atual->falhas);

        fprintf(arquivo, "\tFailrate: %.2f\n", atual->failrate);

        fprintf(arquivo, "\tQuantidade de produtos que entraram: %d\n", atual->qtd_produtos_entraram);

        fprintf(arquivo, "\tQuantidade de produtos concluidos: %d\n", atual->qtd_produtos_concluidos);

        if(!s->resumo || !atual->resumo)
        {
            //fprintf(arquivo, "Nao foi possivel calcular as metricas restantes.\n");
            atual = atual->proxima_etapa;
            continue;
        }

        fprintf(arquivo, "\tMedia de falhas por produto: %.2f\n", atual->resumo->falhas_por_produto);

        fprintf(arquivo, "\tTempo medio: %.2f\n", atual->resumo->tempo_medio);

        fprintf(arquivo, "\t  Tempo minimo: %d\n", atual->resumo->tempo_minimo);

        fprintf(arquivo, "\t  Tempo medio total em filas de atividade: %.2f\n", atual->resumo->tempo_medio_filas_atividades);

        fprintf(arquivo, "\t  Tempo medio na fila de prontos da etapa: %.2f\n", atual->resumo->tempo_medio_fila_prontos);

        fprintf(arquivo, "\tMaior tempo: %d\n", atual->resumo->maior_tempo);

        atual = atual->proxima_etapa;
    }
}

static void imprimir_relatorio_atividades_arquivo(FILE *arquivo, simulacao *s)
{
    if (!arquivo)
    {
        return;
    }

    if (!s || !s->linha)
    {
        fprintf(arquivo, "Linha invalida!\n");
        return;
    }

    if (!s->linha->primeira_etapa)
    {
        fprintf(arquivo, "Nenhuma etapa registrada na linha.\n");
        return;
    }

    fprintf(arquivo, "\n-------------- RELATORIO DE ATIVIDADES --------------\n");

    etapa *e_atual = s->linha->primeira_etapa;

    while (e_atual)
    {
        fprintf(arquivo, "\nETAPA %d, %s:\n", e_atual->id, e_atual->nome);

        atividade *a_atual = e_atual->primeira_atividade;

        while (a_atual)
        {
            fprintf(arquivo, "\n  ATIVIDADE %d, %s:\n", a_atual->id, a_atual->nome);

            fprintf(arquivo, "\tCapacidade: %d \n", a_atual->capacidade_max);

            fprintf(arquivo, "\t  Capacidade por Unidade Funcional: %d\n", a_atual->capacidade_max / a_atual->qtd_uf);

            fprintf(arquivo, "\t  Quantidade de Unidades Funcionais: %d\n", a_atual->qtd_uf);

            fprintf(arquivo, "\tFailrate: %.2f\n", a_atual->failrate);

            if(!s->resumo || !a_atual->resumo)
            {
                //fprintf(arquivo, "Nao foi possivel calcular as metricas restantes.\n");
                a_atual = a_atual->proxima_atividade;
                continue;
            }

            fprintf(arquivo, "\tTempo medio total:  %.2f\n", a_atual->resumo->tempo_medio_total);

            fprintf(arquivo, "\t  Tempo de execucao: %d \n", a_atual->tempo_de_processamento);

            fprintf(arquivo, "\t  Tempo medio em fila: %.2f\n", a_atual->resumo->tempo_medio_na_fila);

            fprintf(arquivo, "\tMaior Tempo: %d \n ", a_atual->resumo->maior_tempo);

            a_atual = a_atual->proxima_atividade;
        }

        e_atual = e_atual->proxima_etapa;
    }
}

static void imprimir_relatorio_produtos_arquivo(FILE *arquivo, simulacao *s)
{
    if (!arquivo)
    {
        return;
    }
    if (!s)
    {
        fprintf(arquivo, "Linha invalida!\n");
        return;
    }
    fprintf(arquivo, "\n-------RELATORIO DE PRODUTOS--------\n");
    mostrar_arvore_arquivo(arquivo, s, s->finalizados);
    fprintf(arquivo, "\nRELATORIO DE PRODUTOS EM LINHA\n");
    mostrar_arvore_arquivo(arquivo, s, s->em_linha);
    
}

static void imprimir_historico_produto_arquivo(FILE *arquivo, simulacao *s, produto *p)
{
    if (!arquivo)
    {
        return;
    }

    if (!p)
    {
        fprintf(arquivo, "Produto nao encontrado\n");
        return;
    }

    fprintf(arquivo, "\n\n---Produto %d---\n\n", p->id);

    fprintf(arquivo, "Modelo: %s\n", s->modelo_produto);

    if (p->tick_saida_linha > 0)
    {
        fprintf(arquivo, "Tick de Criacao: %d \n", p->tick_criacao);
        fprintf(arquivo, "Tick de Entrada na linha: %d \n", p->tick_entrada_linha);
        fprintf(arquivo, "Tick de Saida da linha: %d \n", p->tick_saida_linha);
        fprintf(arquivo, "Localizacao atual: Pilha de Concluidos \n");

        int tempo_filas_atividade = tempo_filas_atividades(p);

        fprintf(arquivo, "Tempo Total no Sistema: %d \n", p->tick_saida_linha - p->tick_criacao);

        fprintf(arquivo, "  Tempo na fila de entrada: %d \n", p->tick_entrada_linha - p->tick_criacao);

        fprintf(arquivo, "  Tempo na linha: %d \n", p->tick_saida_linha - p->tick_entrada_linha);

        fprintf(arquivo, "    Filas de atividade: %d \n", tempo_filas_atividade);

        fprintf(arquivo, "    Processamento e Retrabalho : %d \n", (p->tick_saida_linha - p->tick_entrada_linha) - tempo_filas_atividade);

        fprintf(arquivo, "Tempo Total em espera: %d \n", tempo_filas_atividade + (p->tick_entrada_linha - p->tick_criacao));
    }
    else if (p->tick_saida_linha < 0)
    {
        fprintf(arquivo, "Tick de Criacao: %d \n", p->tick_criacao);
        fprintf(arquivo, "Tick de Entrada na linha: %d \n", p->tick_entrada_linha);
        fprintf(arquivo, "Localizacao atual: Pilha de Lixo \n");
    }
    else if (p->tick_entrada_linha > 0)
    {
        fprintf(arquivo, "Tick de Criacao: %d \n", p->tick_criacao);
        fprintf(arquivo, "Tick de Entrada na linha: %d \n", p->tick_entrada_linha);
        fprintf(arquivo, "Localizacao atual: EM LINHA \n");
    }
    else
    {
        fprintf(arquivo, "Tick de Criacao: %d \n", p->tick_criacao);
        fprintf(arquivo, "Localizacao atual: Fila de entrada da linha \n");
        return;
    }

    fprintf(arquivo, "Falhas: %d \n", p->falhas);

    trajetoria_arquivo(arquivo, p);
}

static void trajetoria_arquivo(FILE *arquivo, produto *p)
{
    if (!arquivo || !p)
    {
        return;
    }

    fprintf(arquivo, "\nTRAJETORIA:\n\n");

    evento_etapa *atual = p->historico_etapas;

    while (atual)
    {
        fprintf(arquivo, "Etapa %d %s tentativa %d\n", atual->e->id, atual->e->nome, atual->tentativa);

        evento_atividade *a_atual = atual->historico_atividades;

        while (a_atual)
        {
            fprintf(arquivo, "Atividade %d %s fila: %d inicio: %d conclusao: %d  %s\n", a_atual->a->id, a_atual->a->nome, a_atual->tick_fila, a_atual->tick_inicio_processamento, a_atual->tick_fim_processamento,  a_atual->falhou ? "FALHOU" : "OK");
            a_atual = a_atual->proximo_evento;
        }

        if (!atual->falhou)
        {
            fprintf(arquivo, "Tempo na etapa: %d\n", atual->tick_fim - atual->tick_inicio);

            fprintf(arquivo, "  Tempo na fila de prontos da etapa: %d\n", atual->tick_fim - atual->tick_conclusao);
        }

        atual = atual->proximo_evento;
    }
}

static void mostrar_arvore_arquivo(FILE *arquivo, simulacao *s , noa *raiz)
{
    if(raiz){
        mostrar_arvore_arquivo(arquivo, s, raiz->esq);
        imprimir_historico_produto_arquivo(arquivo, s, raiz->p);
        mostrar_arvore_arquivo(arquivo, s, raiz->dir);
    }
}

/* ============================================================
   FUNÇÕES AUXILIARES
   ============================================================ */


static int tempo_filas_atividades(produto *p)
{
    int soma = 0;
    evento_etapa *atual = p->historico_etapas;
    while (atual)
    {
        evento_atividade *a_atual = atual->historico_atividades;
        while (a_atual)
        {   
            if(a_atual->tick_fim_processamento != -1)// se a atividade ja foi concluida
                soma += (a_atual->tick_inicio_processamento - a_atual->tick_fila);
            a_atual = a_atual->proximo_evento;
        }
        atual = atual->proximo_evento;
    }
    return soma;
}

static int em_linha(simulacao *s)
{
    if (!s->linha || !s->linha->primeira_etapa)
        return 0;
    int soma = s->fila_entrada? s->fila_entrada->em_fila : 0;
    
    etapa *atual = s->linha->primeira_etapa;
    while (atual)
    {
        soma += atual->ocupacao;
        atual = atual->proxima_etapa;
    }
    return soma;
}

// funcoes auxiliares de imprimir_historico_produto
static void trajetoria(produto * p)
{
    printf("\nTRAJETORIA:\n\n");
    evento_etapa *atual = p->historico_etapas;
    while (atual)
    {
        printf("Etapa %d %s tentativa %d\n", atual->e->id, atual->e->nome, atual->tentativa);
        evento_atividade *a_atual = atual->historico_atividades;
        while (a_atual)
        {   
            if(a_atual->tick_fim_processamento != -1)
                printf("Atividade %d %s fila: %d inicio: %d conclusao: %d  %s\n", a_atual->a->id, a_atual->a->nome, a_atual->tick_fila, a_atual->tick_inicio_processamento, a_atual->tick_fim_processamento, a_atual->falhou ? "FALHOU" : "OK");
            else
                printf("Atividade atual: %d, %s\n", a_atual->a->id, a_atual->a->nome);
            a_atual = a_atual->proximo_evento;
        }
        if(atual->tick_fim > 0){
            printf("Tempo na etapa: %d\n", atual->tick_fim - atual->tick_inicio);
            printf("  Tempo na fila de prontos da etapa: %d\n", atual->tick_fim - atual->tick_conclusao);
        }
        atual = atual->proximo_evento;
    }
}

static void mostrar_arvore(simulacao *s , noa *raiz)
{
    if(raiz){
        mostrar_arvore(s, raiz->esq);
        imprimir_historico_produto(s, raiz->p);
        mostrar_arvore(s, raiz->dir);
    }
}
