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
typedef struct evento evento;


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

    fila * f; //fila de prontos da etapa, onde os produtos ficam esperando para entrar na próxima etapa
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
    
    
    int tick_criacao; //concepção
    int tick_entrada_linha; //entrada na linha de produção
    int tick_saida_linha; //saída da linha de produção
    //tempo total implicito: tick_saida_linha - tick_criacao, não precisa ser armazenado
    
    int tempo_de_espera;
    
    int tick_termino_atividade_atual; //redundante, mas facilita o controle de tempo
    
    int tentativa_atual;

    evento * historico;
    evento * atual;
};



struct evento{
    int id;
    //char descricao[100];?
    int etapa_id;
    int atividade_id;
    int tentativa; //contador
    int falhou; //flag
    int tick_fila; //tick em que o produto entrou na fila para essa atividade,
    int tick_inicio_processamento; //tick em que o produto começou a ser processado nessa atividade, ou seja, quando ele entrou no slot
    int tick_fim_processamento; //tick em que o produto terminou de ser processado nessa atividade, ou seja, quando ele saiu do slot
    
    evento * proximo_evento;
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