#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int produto_id = 1;
int etapa_id = 1;

typedef struct etapa etapa;
typedef struct atividade atividade;
typedef struct produto produto;
typedef estrutura fila fila;
typedef struct etapas etapas;
slot de slot de estrutura typedef;
typedef struct pilha pilha;

fila * fila_entrada = NULL;
pilha * lixo = NULL;
pilha * concluídas = NULL;

struct etapa
{
int id;
nome de caractere[50];
atividade *primeira_atividade;
atividade *última_atividade;
etapa *proxima_etapa;
etapa *etapa_anterior;
fila *f;
int capacidade_max; //somatoria da capacidade de cada atividade + espaço para filas
em ocupação; //número de produtos atualmente na etapa, tanto em atividades quanto em
};
struct atividade
{
int id;
nome de caractere[50];
etapa *etapa_dona;
//produto *produto_execucao;
atividade *proxima_atividade;
fila *f; // fila de prontos para entrar na atividade, entrarão quando tiver capacidade
int capacidade_max;
int ocupação;
slot * slots; //vetor de slots, cada slot representa um produto em execução nessa atividade, o tamanho do vetor é igual à capacidade_max
int tempo_de_processamento; //em ticks
float failrate; //probabilidade de falha, entre 0 e 1
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
struct pilha{
produto *topo;
produto *base;
int em_pilha;
};

void arquivar(produto *p, fila **f){
if (*f == NULL ){
printf("Criando fila de entrada\n");
*f = malloc(tamanho(fila));
if (!*f)
{
printf("\n\nErro ao alocar memória! \n\n");
saída(1);
}
(*f)->inicio = NULL;
(*f)->fim = NULO;
(*f)->em_fila = 0;
}
(*f)->em_fila++;
if ((*f)->inicio == NULL)
{
(*f)->inicio = p;
(*f)->fim = p;
retornar;
}
(*f)->fim->proximo_produto = p;
(*f)->fim = p;

}
void empilhar(produto *p, pilha **pp){
if (*pp == NULL ){
printf("Criando pilha\n");
*pp = malloc(tamanho(pilha));
if (!*pp)
{
printf("\n\nErro ao alocar memória! \n\n");
saída(1);
}
(*pp)->topo = NULL;
(*pp)->base = NULO;
(*pp)->em_pilha = 0;
}
(*pp)->em_pilha++;
if ((*pp)->topo == NULL)
{
(*pp)->topo = p;
(*pp)->base = p;
retornar;
}
p->proximo_produto = (*pp)->topo;
(*pp)->topo = p;

}
void criar_produto(fila **f){
// cria um novo produto, aloca memória, inicializa campos, etc.
produto *novo = malloc(sizeof(produto));
if (!novo)
{
printf("\n\nErro ao alocar memória! \n\n");
saída(1);
}
novo->proximo_produto = NULL;
novo->atividade_atual = NULL;
novo->etapa_atual = NULL;
enfileirar(novo, f); // adiciona produto o na fila de entrada, que pode ser passado como argumento ou ser um ponteiro global, ou algo do tipo
novo->id = produto_id;
novo->defeito = 0;
novo->falhas = 0;
produto_id++;
printf("Produto %d criado e adicionado no final da fila \n", novo->id);
}
produto *desenfileirar(fila *f){
if (f == NULL || f->inicio == NULL)
{
printf("Fila vazia, não e possível desenfileirar produto.\n");
retornar NULO;
}
produto *p = f->inicio;
f->inicio = p->proximo_produto;
if (f->inicio == NULL)
f->fim = NULL;
p->proximo_produto = NULL;
f->em_fila--;
retornar p;
}
void mostrar_fila(fila *f){
// percorre o fila e imprime informações sobre os produtos, como sua etapa atual, atividade atual, etc.
if (!f || !f->inicio)
{
printf("Fila vazia\n");
retornar;
}
produto *atual = f->inicio;
while (atual != NULL)
{
printf("Produto %d ", atual->id);
atual = atual->proximo_produto;
}
printf("\n");
}

void falhar_produto(produto *p, float failrate){
//implementar a verificação de falha, usando a taxa de falha da atividade, ou seja, gerar um número aleatório e comparar com a taxa de falha para determinar se o produto falhou ou não
float r = (float)rand() / RAND_MAX; // gera um número aleatório entre 0 e 1
if (r < failrate)
{
p->falhas++;
if(r < (failrate / 2)){
printf("O produto %d teve uma falha catastrófica na atividade %d\n", p->id, p->atividade_atual->id);
p->defeito = 2;
retornar;
}
printf("Produto %d falhou na atividade %d\n", p->id, p->atividade_atual->id);
p->defeito = 1;
}

}

void criar_slots(atividade *a){
int capacidade = a->capacidade_max;
a->slots = malloc(sizeof(slot) * capacidade);
if (!a->slots)
{
printf("\n\nErro ao alocar memória! \n\n");
saída(1);
}
for (int i = 0; i < capacidade; i++)
{
a->slots[i].p = NULL;
a->slots[i].tempo_restante = 0;
}
}
void criar atividade_(etapa *dona, int indice, int capacidade){
atividade *nova = malloc(sizeof(atividade));
if (!nova)
{
printf("\n\nErro ao alocar memória! \n\n");
saída(1);
}
nova->id = dona->id * 100 + índice;
snprintf(nova->nome, sizeof(nova->nome), "Atividade %d", nova->id);
nova->etapa_dona = dona;
nova->capacidade_max = capacidade;// capacidade arbitrária, pode ser alterada depois
nova->ocupacao = 0;
nova->tempo_de_processamento = 2;
nova->taxa de falha = 0,2; // taxa de falha arbitrária, pode ser alterada depois
nova->f = NULL;
nova->proxima_atividade = NULL;
criar_slots(nova);
printf("Atividade %d criada, apontando os ponteiros\n", nova->id);
if ((dona->primeira_atividade) == NULL)
{
dona->primeira_atividade = nova;
}
else
{
dona->ultima_atividade->proxima_atividade = nova;
}
dona->ultima_atividade = nova;
printf("Ponteiros definidos\n");
}
void mostrar_atividades(atividade *primeira){
atividade *atual = primeira;
while (atual)
{
printf(" %s\n", atual->nome);

    printf("    Fila de entrada: %d produtos\n", (atual->f ? atual->f->em_fila : 0));

    for (int i = 0; i < atual->capacidade_max; i++)
    {
        if (atual->slots[i].p){
            printf("\tSlot %d: Produto %d\n", i, atual->slots[i].p->id);
        }else{
            printf("\tSlot %d: Vazio\n", i);
        }

    }
    
    atual = atual->proxima_atividade;
}
}

void criar_etapa(etapas **e){
etapa *nova = malloc(sizeof(etapa));
if (!nova)
{
printf("\n\nErro ao alocar memória! \n\n");
saída(1);
}
nova->id = etapa_id;
snprintf(nova->nome, sizeof(nova->nome), "Etapa %d", etapa_id); // padrão, pode ser alterado depois
etapa_id++;
nova->primeira_atividade = NULL;
nova->ultima_atividade = NULL;
nova->proxima_etapa = NULL;
nova->ocupação = 0;
nova->capacidade_max = 0;

if (*e == NULL)
{
    *e = malloc(sizeof(etapas));
    if (!*e)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    (*e)->primeira_etapa = nova;
    nova->etapa_anterior = NULL;
}
else
{
    (*e)->ultima_etapa->proxima_etapa = nova;
    nova->etapa_anterior = (*e)->ultima_etapa;
}
(*e)->ultima_etapa = nova;

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
void mostrar_etapas(etapas *e){
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

void pegar_produto_etapa(etapa *e){ //precisa de um ponteiro global pra fila de entrada
//se tiver espaço
produto *p;
if(e->ocupacao >= e->capacidade_max){
printf("Etapa %d cheia\n", e->id);
retornar;
}
if (e->etapa_anterior)
{
printf("Pegando produto da fila de prontos da etapa anterior\n");
p = desenfileirar(e->etapa_anterior->f); // pega o primeiro produto da fila de prontos da etapa anterior, se tiver
}
else
{
printf("Primeira etapa, pegando o produto da fila de entrada\n");
p = desenfileirar(fila_entrada); // pega o primeiro produto da fila de entrada, se tiver
}
if (!p)
{
printf("Nenhum disponível para pegar na etapa %d\n", e->id);
retornar;
}
atividade *a = e->primeira_atividade;
arquivar(p, &a->f); // coloca o produto na fila de entrada da primeira atividade da etapa
p->etapa_atual = e;
p->atividade_atual = a;
e->ocupação++;

if(e->etapa_anterior)
    e->etapa_anterior->ocupacao--;
printf("Produto %d entrou na fila de entrada da Atividade %d\n", p->id, a->id);
}
void pegar_produto_atividade(atividade *a){

if(a->ocupacao >= a->capacidade_max){
        printf("Atividade %d cheia, nao e possivel pegar produto\n", a->id);
        return;
}
produto *p = desenfileirar(a->f); // pega o primeiro produto da fila de entrada da atividade
if(p){
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
void avancar_produto(atividade *a, produto *p){//precisa de um ponteiro global pra pilha de lixo e de concluídos

a->ocupacao--;
if(p->defeituoso){
    if(p->defeituoso == 2){
        printf("Produto %d processado com falha catastrófica na atividade % d, e é considerado defeituoso\n", p->id, a->id);
        empilhar(p, &lixo);
        a->etapa_dona->ocupacao--;
        return;
    }
    printf("Produto %d processado com falha na atividade %d, mas ainda pode ser consertado\n", p->id, a->id);
    p->defeituoso = 0;
    enfileirar(p, &a->etapa_dona->primeira_atividade->f); // coloca o produto de volta na fila de entrada da primeira atividade da etapa, para tentar consertar o produto
    return;
}
    
printf("Produto %d processado com sucesso na atividade %d\n", p->id, a->id);

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
printf("Produto %d concluiu a última etapa do processo\n", p->id);
empilhar(p, &concluidos);
}
void atualizar_atividade(atividade *a){ //atualiza todos os produtos da atividade
//depois refatorar para legibilidade
//implementar verificação de erro? não, implemente na função de avanço, pois o produto sai de qualquer jeito, soq pra tras

for(int i = 0; i < a->capacidade_max; i++){//olha cada slot 
    slot *s = &a->slots[i];
    if(s->p){//se tiver produto no slot
        s->tempo_restante--;//decrementa o tempo dele
        if (s->tempo_restante <= 0)//se o produto terminou de ser processado
        {
            produto *p = s->p;
            printf("Slot %d: Produto %d terminou de ser processado na Atividade %d\n", i, p->id, a->id);
            falhar_produto(p, a->failrate);//verifica se o produto falhou
            avancar_produto(a, p);
            a->slots[i].p = NULL;
        }else{
            printf("Slot %d: Produto em execucao \n", i);    
        }
    }else{
       printf("slot %d: vazio \n", i); 
    }
}
}

void liberar_fila(fila *f){
if (!f)
return;
produto *atual = f->inicio;
while (atual)
{
produto *aux = atual;
atual = atual->proximo_produto;
grátis(aux);
}
grátis(f);
}
void liberar_atividade(atividade *a){
if (!a) return;

// libera produtos nos slots
for (int i = 0; i < a->capacidade_max; i++){
    if (a->slots[i].p){
        free(a->slots[i].p);
        a->slots[i].p = NULL;
    }
}
free(a->slots);
liberar_fila(a->f);
free(a);
}
void liberar_atividades(etapa *e){
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
void liberar_etapas(etapas *e){
etapa *atual = e->primeira_etapa;
while (atual)
{
etapa *aux = atual;
atual = atual->proxima_etapa;
liberar_atividades(aux);
liberar_fila(aux->f);
grátis(aux);
}
grátis(e);
}

void simulador(){
//responsável por controlar o fluxo dos produtos

}

int main(){
//srand(time(NULL));

etapas *e = NULL;

criar_etapa(&e);
criar_etapa(&e);


mostrar_etapas(e);

criar_produto(&fila_entrada);
criar_produto(&fila_entrada);
criar_produto(&fila_entrada);
criar_produto(&fila_entrada);
mostrar_fila(fila_entrada);
/*pegar_produto_etapa(e->primeira_etapa, fila_entrada);

pegar_produto_etapa(e->primeira_etapa, fila_entrada);

pegar_produto_etapa(e->primeira_etapa, fila_entrada);
mostrar_fila(e->primeira_etapa->primeira_atividade->f);
pegar_produto_atividade(e->primeira_etapa->primeira_atividade);
mostrar_etapas(e);
pegar_produto_atividade(e->primeira_etapa->primeira_atividade);
mostrar_etapas(e);
atualizar_atividade(e->primeira_etapa->primeira_atividade);
atualizar_atividade(e->primeira_etapa->primeira_atividade);
pegar_produto_atividade(e->primeira_etapa->primeira_atividade->proxima_atividade);
pegar_produto_atividade(e->primeira_etapa->primeira_atividade->proxima_atividade);
mostrar_etapas(e);
atualizar_atividade(e->primeira_etapa->primeira_atividade->proxima_atividade);
atualizar_atividade(e->primeira_etapa->primeira_atividade->proxima_atividade);
mostrar_etapas(e);
pegar_produto_etapa(e->primeira_etapa->proxima_etapa, fila_entrada);
pegar_produto_etapa(e->primeira_etapa->proxima_etapa, fila_entrada);
mostrar_etapas(e);


//estado atual do programa: não sei, tem que testar tudo pq eu implementei slots e atualizacao de etapa mas não testei
//tb tenho que implementar tempo de execucao e clock
    */
liberar_fila(fila_entrada);
liberar_etapas(e);
printf("Memoria liberada, encerrando programa\n");
return 0;
}
