#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>



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

void inicializar_simulacao(simulacao *s)
{
    s->fila_entrada = NULL;
    s->lixo = NULL;
    s->concluidos = NULL;
    s->linha = NULL;
    s->produto_id = 1;
    s->etapa_id = 1;
    s->produtos_criados = 0;
    s->MODO_MANUAL = 0; // inicia em modo manual, pode ser alterado depois


    s->vazao = 1;
    s->n_produtos_total = 10; //parte arbitrária por enquanto
    s->max_ticks = 100;
}

void enfileirar(produto *p, fila **f)
{
    if (*f == NULL)
    {
        printf("Criando fila de entrada\n");
        *f = malloc(sizeof(fila));
        if (!*f)
        {
            printf("\n\nErro ao alocar memória! \n\n");
            exit(1);
        }
        (*f)->inicio = NULL;
        (*f)->fim = NULL;
        (*f)->em_fila = 0;
    }
    (*f)->em_fila++;
    if ((*f)->inicio == NULL)
    {
        (*f)->inicio = p;
        (*f)->fim = p;
        return;
    }
    (*f)->fim->proximo_produto = p;
    (*f)->fim = p;
}
void empilhar(produto *p, pilha **pp)
{
    if (*pp == NULL)
    {
        printf("Criando pilha\n");
        *pp = malloc(sizeof(pilha));
        if (!*pp)
        {
            printf("\n\nErro ao alocar memória! \n\n");
            exit(1);
        }
        (*pp)->topo = NULL;
        (*pp)->base = NULL;
        (*pp)->em_pilha = 0;
    }
    (*pp)->em_pilha++;
    if ((*pp)->topo == NULL)
    {
        (*pp)->topo = p;
        (*pp)->base = p;
        return;
    }
    p->proximo_produto = (*pp)->topo;
    (*pp)->topo = p;
}
void criar_produto(simulacao *s)
{
    // cria um novo produto, aloca memória, inicializa campos, etc.
    produto *novo = malloc(sizeof(produto));
    if (!novo)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    novo->proximo_produto = NULL;
    novo->atividade_atual = NULL;
    novo->etapa_atual = NULL;
    enfileirar(novo, &s->fila_entrada); // adiciona o produto na fila de entrada, que pode ser passada como argumento ou ser um ponteiro global, ou algo do tipo
    novo->id = s->produto_id;
    novo->defeituoso = 0;
    novo->falhas = 0;

    s->produto_id++;
    s->produtos_criados++;
    printf("Produto %d criado e adicionado no final da fila \n", novo->id);
}
produto *desenfileirar(fila *f)
{
    if (f == NULL || f->inicio == NULL)
    {
        printf("Fila vazia, nao e possivel desenfileirar produto.\n");
        return NULL;
    }
    produto *p = f->inicio;
    f->inicio = p->proximo_produto;
    if (f->inicio == NULL)
        f->fim = NULL;
    p->proximo_produto = NULL;
    f->em_fila--;
    return p;
}
void mostrar_fila(fila *f)
{
    // percorre a fila e imprime informações sobre os produtos, como sua etapa atual, atividade atual, etc.
    if (!f || !f->inicio)
    {
        printf("Fila vazia\n");
        return;
    }
    produto *atual = f->inicio;
    while (atual != NULL)
    {
        printf("Produto %d   ", atual->id);
        atual = atual->proximo_produto;
    }
    printf("\n");
}
void mostrar_pilha(pilha *p, simulacao *s)
{
    printf("Pilha de %s: \n", (p == s->lixo) ? "lixo" : "concluidos");
    if (!p || !p->topo)
    {
        printf("Pilha vazia\n");
        return;
    }
    produto *atual = p->topo;
    while (atual != NULL)
    {
        printf("Produto %d   ", atual->id);
        atual = atual->proximo_produto;
    }
    printf("\n");
}

void falhar_produto(produto *p, float failrate)
{
    // implementar verificação de falha, usando a taxa de falha da atividade, ou seja, gerar um número aleatório e comparar com a taxa de falha para determinar se o produto falhou ou não
    float r = (float)rand() / RAND_MAX; // gera um número aleatório entre 0 e 1
    if (r < failrate)
    {
        p->falhas++;
        if (r < (failrate / 2 || p->falhas > 3))
        { // se for uma falha catastrófica ou se o produto já tiver falhado mais de 3 vezes, considera-se o produto como defeituoso e não tenta consertar mais
            printf("Produto %d teve uma falha catastrófica na atividade %d\n", p->id, p->atividade_atual->id);
            p->defeituoso = 2;
            return;
        }
        printf("Produto %d falhou na atividade %d\n", p->id, p->atividade_atual->id);
        p->defeituoso = 1;
        return;
    }
    p->defeituoso = 0;
    printf("Produto %d processado com sucesso na atividade %d\n", p->id, p->atividade_atual->id);
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
    int qtd_uf = 1; // quantidade de unidades funcionais, arbitraria, pode ser alterada depois
    nova->capacidade_max = capacidade * qtd_uf; // capacidade arbitraria, pode ser alterada depois
    nova->ocupacao = 0;
    nova->tempo_de_processamento = 2;
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
void mostrar_atividades(atividade *primeira)
{
    if (!primeira)
    {
        printf("Nenhuma atividade criada\n");
        return;
    }
    atividade *atual = primeira;
    while (atual)
    {
        printf("    %s\n", atual->nome);

        printf("    Fila de entrada: %d produtos\n", (atual->f ? atual->f->em_fila : 0));

        for (int i = 0; i < atual->capacidade_max; i++)
        {
            if (atual->slots[i].p)
            {
                printf("\tSlot %d: Produto %d\n", i, atual->slots[i].p->id);
            }
            else
            {
                printf("\tSlot %d: Vazio\n", i);
            }
        }

        atual = atual->proxima_atividade;
    }
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
    int num_atividades = 2; // arbitrario
    for (int i = 1; i <= num_atividades; i++)
    {
        int catividade = 2;
        criar_atividade(nova, i, catividade);
        nova->capacidade_max = nova->capacidade_max + catividade;
    }
    int capacidade_filas = 10;
    nova->capacidade_max = nova->capacidade_max + capacidade_filas;
    printf("Etapa %d e suas atividades concluidas\n", nova->id);
}
void mostrar_etapas(etapas *e)
{
    if (!e || !e->primeira_etapa) // nao ta compilando, verificar depois
    {
        printf("Nenhuma etapa criada\n");
        return;
    }
    etapa *atual = e->primeira_etapa;
    while (atual != NULL)
    {
        printf("%s\n", atual->nome);
        mostrar_atividades(atual->primeira_atividade);
        printf("Fila de prontos da etapa %d:\n", atual->id);
        mostrar_fila(atual->f);
        atual = atual->proxima_etapa;
    }
}

void pegar_produto_etapa(etapa *e, fila *fila_entrada)
{ // precisa de um ponteiro global pra fila de entrada
    // se tiver espaço

    if (e->ocupacao >= e->capacidade_max)
    {
        printf("Etapa %d cheia\n", e->id);
        return;
    }
    produto *p;
    if (e->etapa_anterior)
    {
        printf("Pegando produto da fila de prontos da etapa anterior\n");
        p = desenfileirar(e->etapa_anterior->f); // pega o primeiro produto da fila de prontos da etapa anterior, se tiver
        if (p)
        {
            e->etapa_anterior->ocupacao--;
        }
    }
    else
    {
        printf("Primeira etapa, pegando produto da fila de entrada\n");
        p = desenfileirar(fila_entrada); // pega o primeiro produto da fila de entrada, se tiver
    }
    if (!p)
    {
        printf("Nenhum produto disponível para pegar na etapa %d\n", e->id);
        return;
    }
    atividade *a = e->primeira_atividade;
    enfileirar(p, &a->f); // coloca o produto na fila de entrada da primeira atividade da etapa
    p->etapa_atual = e;
    p->atividade_atual = a;
    e->ocupacao++;

    printf("Produto %d entrou na fila de entrada da Atividade %d\n", p->id, a->id);
}
void pegar_produto_atividade(atividade *a)
{

    if (a->ocupacao >= a->capacidade_max)
    {
        printf("Atividade %d cheia, nao e possivel pegar produto\n", a->id);
        return;
    }
    produto *p = desenfileirar(a->f); // pega o primeiro produto da fila de entrada da atividade
    if (p)
    {
        for (int i = 0; i < a->capacidade_max; i++)
        {
            if (!a->slots[i].p)
            {
                a->slots[i].p = p;
                a->slots[i].tempo_restante = a->tempo_de_processamento; // tempo de processamento da atividade, pode ser alterado depois
                a->ocupacao++;
                p->atividade_atual = a;
                printf("Produto %d saiu da fila de entrada e entrou no slot %d da Atividade %d\n", p->id, i, a->id);
                break;
            }
        }

        return;
    }

    printf("Nenhum produto disponivel para pegar na atividade %d\n", a->id);
}
int verificar_defeitos(produto *p, simulacao *s)
{
    if (p->defeituoso)
    {
        if (p->defeituoso == 2)
        {
            printf("Produto %d processado com falha catastrofica e direcionado a pilha de\n", p->id);
            empilhar(p, &s->lixo);
            p->atividade_atual->etapa_dona->ocupacao--;
            return 1;
        }
        printf("Produto %d processado com falha recuperável, redirecionado para o começo da etapa %d\n", p->id, p->atividade_atual->etapa_dona->id);
        p->defeituoso = 0;
        enfileirar(p, &(p->atividade_atual->etapa_dona->primeira_atividade->f)); // coloca o produto de volta na fila de entrada da primeira atividade da etapa, para tentar consertar o produto
        return 1;
    }
    return 0;

}
void avancar_produto(atividade *a, produto *p, simulacao *s)
{ 

    a->ocupacao--;
    if(verificar_defeitos(p, s)) return;
    
    if (a->proxima_atividade)
    {
        enfileirar(p, &a->proxima_atividade->f); // coloca o produto na fila de entrada da proxima atividade,
        printf("Produto %d colocado na fila de entrada da proxima atividade (%d)\n", p->id, a->proxima_atividade->id);
        return;
    }
    printf("Produto %d concluiu a ultima atividade da etapa %d\n", p->id, a->etapa_dona->id);
    if (a->etapa_dona->proxima_etapa)
    {
        printf("Enfileirando produto %d na fila de prontos da etapa\n", p->id);
        enfileirar(p, &a->etapa_dona->f);
        return;
    }
    printf("Produto %d concluiu a ultima etapa do processo\n", p->id);
    empilhar(p, &s->concluidos);
    a->etapa_dona->ocupacao--;
    return;
}
void atualizar_atividade(atividade *a, simulacao *sim)
{ // atualiza todos os produtos da atividade
    // depois refatorar para legibilidade

    for (int i = 0; i < a->capacidade_max; i++)
    { // olha cada slot
        slot *s = &a->slots[i];
        if (s->p)
        { // se tiver produto no slot
            // s->tempo_restante--; recebeu função própria
            if (s->tempo_restante <= 0) // se o produto terminou de ser processado
            {
                produto *p = s->p;
                printf("Slot %d: Produto %d terminou de ser processado na Atividade %d\n", i, p->id, a->id);
                falhar_produto(p, a->failrate); // verifica se o produto falhou
                avancar_produto(a, p, sim);
                a->slots[i].p = NULL; // melhorar legibilidade disso depois
            }
            /*else
            {
                printf("Slot %d: Produto em execucao \n", i);
            }
        }
        else
        {
            printf("slot %d: vazio \n", i);*/
        }
    }
}
void envelhecer_atividade(atividade *a)
{
    for (int i = 0; i < a->capacidade_max; i++)
    {
        slot *s = &a->slots[i];
        if (s->p)
        {
            s->tempo_restante--;
        }
    }
}


void liberar_fila(fila *f)
{
    if (!f)
        return;
    produto *atual = f->inicio;
    while (atual)
    {
        produto *aux = atual;
        atual = atual->proximo_produto;
        free(aux);
    }
    free(f);
}
void liberar_atividade(atividade *a)
{
    if (!a)
        return;

    // libera produtos nos slots
    for (int i = 0; i < a->capacidade_max; i++)
    {
        if (a->slots[i].p)
        {
            free(a->slots[i].p);
            a->slots[i].p = NULL;
        }
    }
    free(a->slots);
    liberar_fila(a->f);
    free(a);
}
void liberar_atividades(etapa *e)
{
    atividade *atual = e->primeira_atividade;
    while (atual)
    {
        atividade *aux = atual;
        atual = atual->proxima_atividade;
        liberar_atividade(aux);
    }
    e->primeira_atividade = NULL;
    e->ultima_atividade = NULL;
}
void liberar_etapas(etapas *e)
{
    etapa *atual = e->primeira_etapa;
    while (atual)
    {
        etapa *aux = atual;
        atual = atual->proxima_etapa;
        liberar_atividades(aux);
        liberar_fila(aux->f);
        free(aux);
    }
    free(e);
}
void liberar_pilha(pilha *p)
{
    if (!p)
        return;
    produto *atual = p->topo;
    while (atual)
    {
        produto *aux = atual;
        atual = atual->proximo_produto;
        free(aux);
    }
    free(p);
}
void encerrar_simulacao(simulacao *s)
{
    printf("Encerrando simulacao\n");
    liberar_fila(s->fila_entrada);
    liberar_etapas(s->linha);
    liberar_pilha(s->lixo);
    liberar_pilha(s->concluidos);
    exit(0);
}

etapa *buscar_etapa(etapas *e, int id)
{
    if (!e)
        return NULL;
    etapa *atual = e->primeira_etapa;
    while (atual && atual->id != id)
    {
        atual = atual->proxima_etapa;
    }
    return atual; // ou devolve nulo ou devolve a etapa encontrada
}
atividade *buscar_atividade(etapas *e, int id)
{
    etapa *atual_etapa = buscar_etapa(e, id / 100); // considerando que o id da atividade é formado pelo id da etapa seguido de um número sequencial, ex: etapa 1 tem atividades 101, 102, etc.
    if (!atual_etapa)
        return NULL;
    atividade *atual = atual_etapa->primeira_atividade;
    while (atual && atual->id != id)
    {
        atual = atual->proxima_atividade;
    }
    return atual; // ou devolve nulo ou devolve a atividade encontrada
}

void contagem_regressiva( const char *mensagem)
{
    printf("%s\n", mensagem);
    Sleep(500);
    printf("3\n");
    Sleep(1000);
    printf("2\n");
    Sleep(1000);
    printf("1\n");
    Sleep(1000);
}

void opcoes(simulacao *s)
{
    while (1)
    {
        int opcao;
        printf("Opcoes:\n");
        printf("1 - Criar produto\n");
        printf("2 - Criar etapa\n");
        printf("3 - Mostrar fila\n");
        printf("4 - Mostrar etapas e atividades\n");
        printf("5 - Pegar_produto_Atividade\n");
        printf("6 - Atualizar atividade\n");
        printf("7 - Pegar_produto_Etapa\n");
        printf("8 - Mostrar pilha\n");
        printf("9 - Continuar simulacao\n");
        printf("10 - Alternar modo manual/automatico (atualmente %s)\n", s->MODO_MANUAL ? "manual" : "automatico");
        printf("11 - Sair\n");
        
        scanf(" %d", &opcao);
        switch (opcao)
        {
        
        case 1:
            criar_produto(s);
            break;
        case 2:
            criar_etapa(s);
            break;
        case 3:
        {
            int op_fila;
            printf("Mostrar qual fila?\n");
            printf("1 - Fila de entrada\n");
            printf("2 - Fila de prontos de etapa\n");
            printf("3 - Fila de entrada de atividade\n");
            printf("4 - Voltar\n");
            scanf("%d", &op_fila);
            switch (op_fila)
            {
            case 1:
                mostrar_fila(s->fila_entrada);
                break;
            case 2:
            {
                int id;
                printf("Digite o id da etapa: ");
                scanf("%d", &id);
                etapa *et = buscar_etapa(s->linha, id);
                if (et)
                    mostrar_fila(et->f);
                else
                    printf("Etapa nao encontrada\n");
                break;
            }
            case 3:
            {
                int id;
                printf("Digite o id da atividade: ");
                scanf("%d", &id);
                atividade *a = buscar_atividade(s->linha, id);
                if (a)
                    mostrar_fila(a->f);
                else
                    printf("Atividade nao encontrada\n");
                break;
            }
            case 4:
                break;
            default:
                printf("Opcao invalida\n");
            }
        }
        break;
        case 4:
            mostrar_etapas(s->linha);
            break;
        case 5:
        {
            int id;
            printf("Digite o id da atividade: ");
            scanf("%d", &id);
            atividade *a = buscar_atividade(s->linha, id);
            if (a)
                pegar_produto_atividade(a);
            else
                printf("Atividade nao encontrada\n");
        }
        break;
        case 6:
        {
            int id;
            printf("Digite o id da atividade: ");
            scanf("%d", &id);
            atividade *a = buscar_atividade(s->linha, id);
            if (a)
                atualizar_atividade(a, s);
            else
                printf("Atividade nao encontrada\n");
        }
        break;
        case 7:
        {
            int id;
            printf("Digite o id da etapa: ");
            scanf("%d", &id);
            etapa *et = buscar_etapa(s->linha, id);
            if (et)
                pegar_produto_etapa(et, s->fila_entrada);
            else
                printf("Etapa nao encontrada\n");
        }
        break;
        case 8:
        {
            int op;
            printf("Pilha de concluidos ou lixo? (1 - concluidos, 2 - lixo): ");
            scanf("%d", &op);
            mostrar_pilha(op == 1 ? s->concluidos : s->lixo, s);
        }
        break;
        case 9:
        {
            contagem_regressiva("Continuando simulacao em");
            return;
        }
        break;
        case 10:{
            s->MODO_MANUAL = !s->MODO_MANUAL;
            printf("Modo manual %s\n", s->MODO_MANUAL ? "ativado" : "desativado");
        }
        break;
        case 11:
        {
            encerrar_simulacao(s);
        }
            exit(0);
        default:
        {
            printf("Opcao invalida\n");
            break;
        }
        }
    }
}

int linha_vazia(simulacao *s)
{
    if (!s->linha || !s->linha->primeira_etapa)
        return 1;
    if (s->fila_entrada && s->fila_entrada->em_fila > 0)
        return 0;
    etapa *atual = s->linha->primeira_etapa;
    while (atual)
    {
        if (atual->ocupacao > 0)
            return 0;
        atual = atual->proxima_etapa;
    }
    return 1;
}
int ha_produtos_para_entrar_etapa(etapa *e, simulacao *s)
{
    etapa *etapa_origem = e->etapa_anterior;
    if (!etapa_origem)
        return (s->fila_entrada && s->fila_entrada->em_fila > 0);
    return (etapa_origem->f && etapa_origem->f->em_fila > 0);
}
int ha_produtos_para_entrar_atividade(atividade *a)
{
    return (a->f && a->f->em_fila > 0);
}
void envelhecer_produtos(simulacao *s)
{
    etapa *atual_etapa = s->linha->primeira_etapa;
    while (atual_etapa)
    {
        atividade *atual_atividade = atual_etapa->primeira_atividade;
        while (atual_atividade)
        {
            envelhecer_atividade(atual_atividade);
            atual_atividade = atual_atividade->proxima_atividade;
        }
        atual_etapa = atual_etapa->proxima_etapa;
    }
}
void saidas(simulacao *s)
{
    etapa *atual_etapa = s->linha->primeira_etapa;
    while (atual_etapa)
    {
        atividade *atual_atividade = atual_etapa->primeira_atividade;
        while (atual_atividade)
        {
            atualizar_atividade(atual_atividade, s);
            atual_atividade = atual_atividade->proxima_atividade;
        }
        atual_etapa = atual_etapa->proxima_etapa;
    }
}
void entradas(simulacao *s)
{
    etapa *atual_etapa = s->linha->ultima_etapa;
    while (atual_etapa)
    {
        while (atual_etapa->ocupacao < atual_etapa->capacidade_max && ha_produtos_para_entrar_etapa(atual_etapa, s))
        {
            pegar_produto_etapa(atual_etapa, s->fila_entrada);
        }
        atividade *atual_atividade = atual_etapa->primeira_atividade;
        while (atual_atividade) // para cada atividade da etapa (qualquer ordem):
        {
            while (atual_atividade->ocupacao < atual_atividade->capacidade_max && ha_produtos_para_entrar_atividade(atual_atividade))
            {
                pegar_produto_atividade(atual_atividade);
            }
            atual_atividade = atual_atividade->proxima_atividade;
        }
        atual_etapa = atual_etapa->etapa_anterior; // percorre as etapas em ordem reversa
    }
}
void popular(simulacao *s)
{

    for (int i = 0; i < s->vazao; i++) // cria v produtos por tick, no momento vazao precisa ser um inteiro igual ou maior 1
    {
        if (s->produtos_criados >= s->n_produtos_total)
        {
            return;
        }
        criar_produto(s);
    }
}

void simular(simulacao *s)
{
    printf("\n\nQuando em simulacao, pressione 'p' para acessar as opcoes\n");
    Sleep(500);
    contagem_regressiva("Iniciando simulacao em");
    if (s->MODO_MANUAL)
    {
        printf("Modo manual ativado, pressione 'E' para iniciar a simulacao \n");
        while (1)
        {
            char c = _getch();
            if (c == 'E' || c == 'e')
            {

                break;
            }
        }
    }

    int tick = 0;
    while (tick < s->max_ticks)
    {
        tick++;
        printf("\nTick %d\n", tick);
        if (_kbhit())
        {                      // Verifica se uma tecla foi pressionada
            char c = _getch(); 
            if (c == 'p' || c == 'P')
            {             // Se for 'p' ou 'P', pausa e exibe as opções
                opcoes(s); // posteriormente implementar uma função melhor de exibição da situação atual do programa
            }
            
        }

        // popula a fila de entrada com novos produtos
        printf("fase de populacao da fila de entrada\n");
        popular(s); 

        if (linha_vazia(s))
        {
            printf("Linha de producao vazia, encerrando simulacao\n");
            break;
        }

        // decrementa o tempo restante de todos os produtos em execução
        printf("fase de envelhecimento dos produtos\n");
        envelhecer_produtos(s);

        // resolve as saidas de todos os produtos que terminaram de ser processados
        printf("fase de saida dos produtos\n");
        saidas(s);

        // admite em execução todos os produtos que puderem entrar, tanto nas atividades quanto nas etapas
        printf("fase de entrada dos produtos\n");
        entradas(s);

        if (s->MODO_MANUAL)
        {
            printf("Pressione 'E' para continuar para o proximo tick ou 'P' para acessar as opcoes \n");

            while (1)
            {
                char c = _getch();
                if (c == 'E' || c == 'e')
                {
                    break;
                }
                if (c == 'p' || c == 'P')
                {             // Se for 'p' ou 'P', pausa e exibe as opções
                    opcoes(s); // posteriormente implementar uma função melhor de exibição da situação atual do programa
                    break;
                }
            }
        }else
            Sleep(1000); // periodo do clock em milisegundos, pode ser alterado depois
    }
}

int main()
{
    srand(time(NULL));
    simulacao s;
    inicializar_simulacao(&s);
    printf("Modo manual(1) ou automatico(0)?\n");
    scanf("%d", &s.MODO_MANUAL);

    criar_etapa(&s);
    criar_etapa(&s);
    criar_etapa(&s);

    simular(&s);

    mostrar_pilha(s.concluidos, &s);
    mostrar_pilha(s.lixo, &s);

    encerrar_simulacao(&s);
}
