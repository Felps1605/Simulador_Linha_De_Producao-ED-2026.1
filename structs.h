#ifndef STRUCT_H
#define STRUCT_H

typedef struct etapa etapa;
typedef struct atividade atividade;
typedef struct produto produto;
typedef struct fila fila;
typedef struct etapas etapas;
typedef struct slot slot;
typedef struct pilha pilha;
typedef struct simulacao simulacao;
typedef struct evento_atividade evento_atividade;
typedef struct evento_etapa evento_etapa;
typedef struct resumo_simulacao resumo_simulacao;
typedef struct resumo_etapa resumo_etapa;
typedef struct resumo_atividade resumo_atividade;
typedef struct noa noa;


struct simulacao{
    // Configuracao
    int    semente;
    char   nome_cenario[100];
    char   arquivo_entrada[100];
    char   id_simulacao[50];
    int    n_produtos_total;
    int    vazao;
    char   modelo_produto[100];
    int    max_ticks;
    int  n_etapas;
    float meta; // != n° total de produtos. não é usado em cálculos, é um valor somente de expectativa
    //por enquanto é uma porcentagem dos produtos totais

    // Estado
    int    tick_atual;
    int    produtos_criados;
    int    produtos_concluidos;
    int    falhas_totais;
    int    produto_id;
    int    etapa_id;
    int    MODO_MANUAL;
    int    tempo_total_espera_produtos;
    int  tempo_excedido;
    //com a soma de todos os tempos de espera de cada um dos produtos é só dividir pelo
    // n de produtos concluidos para ter o tempo medio de espera

    // Estruturas
    etapas * linha;         
    fila * fila_entrada;
    pilha * concluidos;
    pilha * lixo;
    noa * em_linha;
    noa * finalizados;
    
    resumo_simulacao * resumo;

};

struct etapa{
    int id;
    char nome[50];
    int num_atividades;
    int capacidade_max; //somatoria da capacidade de cada atividade + espaço para filas
    int ocupacao; //número de produtos atualmente na etapa, tanto em atividades quanto em filas
    float failrate;

    etapa * proxima_etapa;
    etapa * etapa_anterior;     
    
    atividade * primeira_atividade;
    atividade * ultima_atividade;

    fila * f; //fila de prontos da etapa

    int falhas;
    int qtd_produtos_concluidos;//incrementar quando um produto sai
    int qtd_produtos_entraram;//incrementar quando um produto entra

    resumo_etapa * resumo;
    
};

struct atividade{
    int id;
    char nome[50];
    int tempo_de_processamento; //em ticks
    float failrate; //probabilidade de falha, entre 0 e 1
    int capacidade_max; //capacidade por unidade funcional * quantidade de unidades funcionais, ou seja, o número máximo de produtos que podem ser processados simultaneamente nessa atividade
    int qtd_uf;
    int ocupacao; //número de produtos atualmente sendo processados nessa atividade  
    
    slot * slots; //vetor de slots, cada slot representa um produto em execução nessa atividade, o tamanho do vetor é igual à capacidade_max
    
    etapa * etapa_dona;

    atividade * proxima_atividade;

    resumo_atividade * resumo;

    fila * f; //fila de espera para essa atividade
};

struct produto{
    int id;
    
    int defeituoso; //flag
    int falhas; //contador

    etapa * etapa_atual;

    atividade * atividade_atual;

    produto *proximo_produto;
     
    
    int tick_criacao; 
    int tick_entrada_linha; 
    int tick_saida_linha; 

     
    //int tempo_total_em_espera;//somatoria dos tempos de espera totais de cada etapa e na fila inicial
    //também é possivel calcular o tempo total em espera subtraindo o tempo total real do tempo total mínimo
    

    evento_etapa * historico_etapas;//primeiro da lista/fila
    evento_etapa * evento_atual_etapa;//ultimo da lista/fila
};

struct evento_atividade{
    
    atividade * a;
    int falhou; 
    int tick_fila; //tick em que o produto entrou na fila para essa atividade 
    int tick_inicio_processamento; //tick em que o produto começou a ser processado nessa atividade 
    int tick_fim_processamento; //tick em que o produto terminou de ser processado nessa atividade
    //usar para calcular:
    //tempo total na atividade
    //tempo na fila

    evento_atividade * proximo_evento;
};

struct evento_etapa{
    
    etapa * e;
    int tentativa; //contador 
    int falhou; //flag 
    
    int tick_inicio; //tick em que o produto entrou na etapa
    int tick_conclusao;//tick em que o produto foi pra fila de prontos
    int tick_fim; //tick em que o produto saiu da etapa
    
    //usar para calcular:
    //tempo total na etapa
    //tempo total na fila de prontos da etapa
    

    evento_etapa * proximo_evento;

    evento_atividade * historico_atividades;//primeiro da lista/fila
    evento_atividade * evento_atual_atividade;//ultimo da lista/fila

    //calcular somatoria de todos os tempos de espera na etapa.(filas de atividades + fila de prontos)
};

struct fila
{
    produto *inicio;
    produto *fim;
    int em_fila;
};

struct etapas
{
    etapa *primeira_etapa;
    etapa *ultima_etapa;
};

struct slot
{
    produto *p;
    int tempo_restante;
};

struct pilha
{
    produto *topo;
    produto *base;
    int em_pilha;
};

struct noa
{
    produto *p;
    noa * esq;
    noa * dir;
};

/*---------------
STRUCTS DE RESUMO
-----------------*/

struct resumo_simulacao{
    simulacao * s;

    float tempo_medio_total;
    float tempo_medio_em_espera;
    float tempo_medio_filas_atividades;
    float tempo_medio_na_fila_entrada;
    float tempo_medio_retrabalho;
    int tempo_minimo;
    int meta_alcancada;

    resumo_etapa * resumos_etapas;

};

struct resumo_etapa{
    etapa * e;

    float soma_tempo_fila_prontos;
    float soma_tempo_total;

    int passagens;
    float falhas_por_produto;
    int tempo_minimo;
    float tempo_medio;
    int maior_tempo;
    float tempo_medio_filas_total;
    float tempo_medio_filas_atividades;
    float tempo_medio_fila_prontos;

    resumo_atividade * resumos_atividades;

};

struct resumo_atividade{
    atividade * a;

    float soma_tempo_na_fila;
    float soma_tempo_na_fila_etapa_valida;
    float soma_tempo_total;

    int passagens;
    float tempo_medio_na_fila;
    float tempo_medio_na_fila_etapas_validas;
    int maior_tempo;
    float tempo_medio_total;

};


#endif
