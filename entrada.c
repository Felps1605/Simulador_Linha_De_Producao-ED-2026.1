#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // necessário para usar time()
#include "entrada.h"

// Funções auxiliares
int criar_slots(atividade *a)
{
    a->slots = malloc(sizeof(slot) * a->capacidade_max);
    if (!a->slots)
    {
        printf("Erro ao alocar memória!\n");
        return 0;
    }
    for (int i = 0; i < a->capacidade_max; i++)
    {
        a->slots[i].p = NULL;
        a->slots[i].tempo_restante = 0;
    }
    return 1;
}

int criar_atividade(etapa *dona, int indice, char *nome, int tempo, float failrate, int capacidade_uf, int qtd_uf)
{   
    atividade *nova = malloc(sizeof(atividade));
    if (!nova)
    {
        printf("Erro ao alocar memória!\n");
        return 0;
    }
    nova->id = dona->id * 100 + indice;
    
    if ( tempo <= 0 || failrate < 0.0 || failrate > 1.0|| capacidade_uf <= 0 || qtd_uf <= 0)
    {
        printf("Erro: dados inválidos na ATIVIDADE %d.\n", nova->id);
        free(nova);
        return 0;
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
    nova->slots = NULL;
    nova->resumo = NULL;

    if(!criar_slots(nova))
    {
        free(nova);
        return 0;
    }
        

    if (dona->primeira_atividade == NULL)
        dona->primeira_atividade = nova;
    else
        dona->ultima_atividade->proxima_atividade = nova;
    dona->ultima_atividade = nova;

    return 1;
}

etapa * criar_etapa(simulacao *s, char *nome, int capacidade, int qtdAtividades, float failrate)
{  
    etapa *nova = malloc(sizeof(etapa));
    if (!nova)
    {
        printf("Erro ao alocar memória!\n");
        return NULL;
    }
    nova->id = s->etapa_id;
    s->etapa_id++;
    if ( capacidade < 0 || qtdAtividades <= 0)
    {
        printf("Erro: dados inválidos na ETAPA %d.\n", nova->id);
        free(nova);
        return NULL;
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
    
    nova->f = malloc(sizeof(fila));
    if(!nova->f){
            printf("Erro ao alocar memória!\n");
            free(nova);
            return NULL;
        }
    nova->f->inicio = NULL;
    nova->f->fim = NULL;
    nova->f->em_fila = 0;

    if (s->linha == NULL)
    {
        s->linha = malloc(sizeof(etapas));
        if(!s->linha){
            printf("Erro ao alocar memória!\n");
            free(nova->f);
            free(nova);
            return NULL;
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
    s->MODO_MANUAL = 0; 

    s->tempo_total_espera_produtos = 0;
    s->tick_atual = 0;
    strcpy(s->arquivo_entrada, "arquivo.txt");

    // Valores padrão (podem ser sobrescritos pelo arquivo)
    /*s->vazao = 2;
    s->n_produtos_total = 20;
    s->max_ticks = 100000;
    s->semente = time(NULL);*/// semente aleatória baseada no tempo atual, sobrescrita pelo arquivo por enquanto
   
    s->n_etapas = 0;//vai ser sobreescrito, é somente para segurança
    s->tempo_excedido = 0;
    s->resumo = NULL;
    s->em_linha = NULL;
    s->finalizados = NULL;
    
}
// Função principal de leitura do arquivo
int lerEntrada( FILE *arquivo, simulacao *s)
{
    char linha[200];

    // Primeiro inicializa a simulação com valores padrão
    inicializar_simulacao(s);

    // Ignora primeira linha
    if (!fgets(linha, sizeof(linha), arquivo))
    {
        printf("Erro: arquivo vazio.\n");
        return 0;
    }

    // SIMULACAO
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "SIMULACAO %99s %d %d", s->nome_cenario, &s->semente, &s->max_ticks) != 3)
    {
        printf("Erro: linha SIMULACAO inválida.\n");
        return 0;
    }
    if (s->semente < 0 || s->max_ticks <= 0)
    {
        printf("Erro: valores inválidos em SIMULACAO.\n");
        return 0;
    }

    // PRODUTOS
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "PRODUTOS %d %d %f %99s", &s->n_produtos_total, &s->vazao, &s->meta, s->modelo_produto) != 4)
    {
        printf("Erro: linha PRODUTOS inválida.\n");
        return 0;
    }
    
    if (s->n_produtos_total <= 0 || s->vazao <= 0)
    {
        printf("Erro: valores inválidos em PRODUTOS.\n");
        return 0;
    }
    if(s->meta > s->n_produtos_total){
        printf("Erro: Meta inalcancavel (meta(%.1f) > quantidade de produtos(%d))\n", s->meta, s->n_produtos_total);
        return 0;
    }

    // LINHA_PRODUCAO
    if (!fgets(linha, sizeof(linha), arquivo) ||
        sscanf(linha, "LINHA_PRODUCAO %d", &s->n_etapas) != 1 || s->n_etapas <= 0)
    {
        printf("Erro: linha LINHA_PRODUCAO inválida.\n");
        return 0;
    }

    for (int i = 0; i < s->n_etapas; i++)
    {
        int qtdAtividades, capacidade;
        float taxaFalha;
        char nomeEtapa[50];

        if (!fgets(linha, sizeof(linha), arquivo) ||
            sscanf(linha, "ETAPA %d %d %f %49s", &qtdAtividades, &capacidade, &taxaFalha, nomeEtapa) != 4)
        {
            printf("Erro: linha ETAPA inválida.\n");
            return 0;
        }
        if (capacidade <= 0 || taxaFalha < 0.0 || taxaFalha > 1.0 || qtdAtividades <= 0)
        {
            printf("Erro: dados inválidos na ETAPA %d.\n", i + 1);
            return 0;
        }
        

        etapa * e = criar_etapa(s, nomeEtapa, capacidade, qtdAtividades, taxaFalha);
        if(!e)
            return 0;

        for (int j = 1; j <= qtdAtividades; j++)
        {
            int tempo, capacidadeuf, qtdUF;
            float taxaFalhaA;
            char nomeAtividade[50];

            if (!fgets(linha, sizeof(linha), arquivo) ||
                sscanf(linha, "ATIVIDADE %d %f %d %d %49s", &tempo, &taxaFalhaA, &capacidadeuf, &qtdUF, nomeAtividade) != 5)
            {
                printf("Erro: linha ATIVIDADE inválida.\n");
                return 0;
            }
            if (tempo <= 0 || taxaFalhaA < 0.0 || taxaFalhaA > 1.0 ||capacidadeuf <= 0 || qtdUF <= 0)
            {
                printf("Erro: dados inválidos na ATIVIDADE.\n");
                return 0;
            }

            if(!criar_atividade(e, j, nomeAtividade, tempo, taxaFalhaA, capacidadeuf, qtdUF))
                return 0;
        }
    }
    return 1;
}



