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


struct simulacao{
    // Configuracao
    int    semente;
    char   nome_cenario[100];
    char   arquivo_entrada[100];
    char   id_simulacao[100];
    int    n_produtos_total;
    int    vazao;
    char   modelo_produto[100];
    int    max_ticks;


    // Estado
    int    tick_atual;
    int    produtos_criados;
    int    produtos_concluidos;
    int    falhas_totais;
    int    produto_id;
    int    etapa_id;
    int    MODO_MANUAL;
    int    tempo_total_espera_produtos;
    //com a soma de todos os tempos de espera de cada um dos produtos é só dividir pelo
    // n de produtos concluidos para ter o tempo medio de espera

    // Estruturas
    etapas * linha;         
    fila * fila_entrada;
    pilha * concluidos;
    pilha * lixo;
    //TabelaHash       hash_produtos
    //NoBST*           bst_concluidos  (ponteiro pra raiz)

};

struct etapa{
    int id;
    char nome[50];
    int num_atividades;
    int capacidade_max; //somatoria da capacidade de cada atividade + espaço para filas
    int ocupacao; //número de produtos atualmente na etapa, tanto em atividades quanto em filas
    
    etapa * proxima_etapa;
    etapa * etapa_anterior;     
    
    atividade * primeira_atividade;
    atividade * ultima_atividade;

    fila * f; //fila de prontos da etapa

    int falhas;
    int qtd_produtos_concluidos;//incrementar quando um produto sai
    int qtd_produtos_entraram;//incrementar quando um produto entra
    
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
    //tempo total implicito: tick_saida_linha - tick_criacao, não precisa ser armazenado
     
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

/*
typedef struct etapa etapa;
typedef struct atividade atividade;
typedef struct produto produto;
typedef struct fila fila;
typedef struct etapas etapas;
typedef struct slot slot;
typedef struct pilha pilha;
typedef struct simulacao simulacao;

struct simulacao
{
    fila *fila_entrada;
    pilha *lixo;
    pilha *concluidos;
    etapas * linha; //etapas *g_e = NULL;

    int produto_id;
    int etapa_id;
    int produtos_criados;
    int MODO_MANUAL;

    int    semente;
    char   nome_cenario[100];
    char   arquivo_entrada[100];
    char   id_simulacao[100];
    int    n_produtos_total;
    int    vazao;
    char   modelo_produto[100];
    int    max_ticks;

    // Estado
    int    tick_atual;
    int    produtos_concluidos;
    int    falhas_totais;
    
    

    //TODA E QUALQUER FUNÇÃO QUE USE UMA "VARIAVEL GLOBAL": g_e, lixo, concluidos, fila de entrada, tick, ids e n de criados etc PRECISA SER REVISADA
    //NECESSÁRIO UMA CAÇA A QUALQUER REFERENCIA AO SISTEMA ANTIGO DE VARIÁVEIS GLOBAIS
    //REVISAR FUNÇAO POR FUNCAO
    
    //será q vale a pena fazer um ponteiro global para a simulaçao?
};
struct etapa
{
    int id;
    char nome[50];
    atividade *primeira_atividade;
    atividade *ultima_atividade;
    etapa *proxima_etapa;
    etapa *etapa_anterior;
    fila *f;
    int capacidade_max; // somatoria da capacidade de cada atividade + espaço para filas
    int ocupacao;       // número de produtos atualmente na etapa, tanto em atividades quanto em
    //int num_atividades;
};
struct atividade
{
    int id;
    char nome[50];
    etapa *etapa_dona;
    // produto *produto_execucao;
    atividade *proxima_atividade;
    fila *f; // fila de prontos para entrar na atividade, entrarão quando tiver capacidade
    int capacidade_max;
    int ocupacao;
    slot *slots;                // vetor de slots, cada slot representa um produto em execução nessa atividade, o tamanho do vetor é igual à capacidade_max
    int tempo_de_processamento; // em ticks
    float failrate;             // probabilidade de falha, entre 0 e 1
    int qtd_uf;
};
struct produto
{
    int id;
    produto *proximo_produto;
    atividade *atividade_atual;
    etapa *etapa_atual;
    int defeituoso;
    int falhas;
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
*/

#endif