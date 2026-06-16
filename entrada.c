#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // necessário para usar time()
#include "entrada.h"

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

void criar_atividade(etapa *dona, int indice, char *nome, int tempo, float failrate, int capacidade_uf, int qtd_uf)
{   
    atividade *nova = malloc(sizeof(atividade));
    if (!nova)
    {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }
    nova->id = dona->id * 100 + indice;

    if ( tempo <= 0 || failrate < 0.0 || failrate > 1.0)
    {
        printf("Erro: dados inválidos na ATIVIDADE %d.\n", nova->id);
        exit(1);
    }

    
    strcpy(nova->nome, nome);
    nova->etapa_dona = dona;
    nova->qtd_uf = qtd_uf;
    nova->capacidade_max = capacidade_uf * qtd_uf;
    nova->ocupacao = 0;
    nova->tempo_de_processamento = tempo;
    nova->failrate = failrate;
    nova->f = NULL;
    nova->proxima_atividade = NULL;

    nova->resumo = NULL;

    criar_slots(nova);

    if (dona->primeira_atividade == NULL)
        dona->primeira_atividade = nova;
    else
        dona->ultima_atividade->proxima_atividade = nova;
    dona->ultima_atividade = nova;

    //return nova;
}

etapa * criar_etapa(simulacao *s, char *nome, int capacidade, int qtdAtividades, float failrate)
{  
    etapa *nova = malloc(sizeof(etapa));
    if (!nova)
    {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }
    nova->id = s->etapa_id;
    s->etapa_id++;
    if ( capacidade < 0 || qtdAtividades <= 0)
    {
        printf("Erro: dados inválidos na ETAPA %d.\n", nova->id);
        exit(1);
    }
    
    strcpy(nova->nome, nome);
    nova->num_atividades = qtdAtividades;
    nova->primeira_atividade = NULL;
    nova->ultima_atividade = NULL;
    nova->proxima_etapa = NULL;
    nova->ocupacao = 0;
    nova->capacidade_max = capacidade;
    nova->failrate = failrate;
    nova->qtd_produtos_concluidos = 0;
    nova->qtd_produtos_entraram = 0;
    nova->falhas = 0;

    nova->resumo = NULL;

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


void gerar_id_simulacao(simulacao *s)
{
    time_t agora;
    struct tm *info_tempo;

    time(&agora);
    info_tempo = localtime(&agora);

    strftime(s->id_simulacao, sizeof(s->id_simulacao),"%Y%m%d_%H%M%S", info_tempo);
}

// Função para inicializar a simulação com valores padrão
void inicializar_simulacao(simulacao *s)
{   
    gerar_id_simulacao(s); //s->id_simulacao = horario
    s->fila_entrada = NULL;
    s->lixo = NULL;
    s->concluidos = NULL;
    s->linha = NULL;
    s->produto_id = 1;
    s->etapa_id = 1;
    s->produtos_criados = 0;
    s->produtos_concluidos = 0;
    s->falhas_totais = 0;
    s->MODO_MANUAL = 0; // inicia em modo manual

    s->tempo_total_espera_produtos = 0;
    s->tick_atual = 0;
    strcpy(s->arquivo_entrada, "arquivo.txt");

    // Valores padrão (podem ser sobrescritos pelo arquivo)
    s->vazao = 2;
    s->n_produtos_total = 20;
    s->max_ticks = 100000;
    s->semente = time(NULL);// semente aleatória baseada no tempo atual, sobrescrita pelo arquivo por enquanto
   
    s->tempo_excedido = 0;
    s->resumo = NULL;
    s->em_linha = NULL;
    s->finalizados = NULL;
    
}
// Função principal de leitura do arquivo
void lerEntrada( FILE *arquivo, simulacao *s)
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
    s->meta = (float)s->n_produtos_total *0.5;
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
    s->n_etapas = qtdEtapas;

    for (int i = 0; i < qtdEtapas; i++)
    {
        int qtdAtividades, capacidade;
        float taxaFalha;
        char nomeEtapa[50];

        if (!fgets(linha, sizeof(linha), arquivo) ||
            sscanf(linha, "ETAPA %d %d %f %49s", &qtdAtividades, &capacidade, &taxaFalha, nomeEtapa) != 4)
        {
            printf("Erro: linha ETAPA inválida.\n");
            exit(1);
        }
        if (capacidade < 0 || taxaFalha < 0.0 || taxaFalha > 1.0 || qtdAtividades <= 0)
        {
            printf("Erro: dados inválidos na ETAPA %d.\n");
            exit(1);
        }
        

        etapa * e = criar_etapa(s, nomeEtapa, capacidade, qtdAtividades, taxaFalha);

        for (int j = 1; j <= qtdAtividades; j++)
        {
            int tempo, capacidadeuf, qtdUF;
            float taxaFalhaA;
            char nomeAtividade[50];

            if (!fgets(linha, sizeof(linha), arquivo) ||
                sscanf(linha, "ATIVIDADE %d %f %d %d %49s", &tempo, &taxaFalhaA, &capacidadeuf, &qtdUF, &nomeAtividade) != 5)
            {
                printf("Erro: linha ATIVIDADE inválida.\n");
                exit(1);
            }
            if (tempo <= 0 || taxaFalhaA < 0.0 || taxaFalhaA > 1.0 ||capacidadeuf < 0 || qtdUF < 0)
            {
                printf("Erro: dados inválidos na ATIVIDADE.\n");
                exit(1);
            }

            criar_atividade(e, j, nomeAtividade, tempo, taxaFalhaA, capacidadeuf, qtdUF);
        }
    }
}



