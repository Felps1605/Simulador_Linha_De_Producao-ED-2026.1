#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#include "structs.h"
#include "execucao.h"
#include "output.h"

/*-----------------------
  PROTOTIPOS PRIVADOS
  ----------------------*/

void enfileirar(produto *p, fila **f);
void empilhar(produto *p, pilha **pp, simulacao *s);
void criar_produto(simulacao *s);
produto *desenfileirar(fila *f);
void mostrar_fila(fila *f);
void falhar_produto(produto *p, float failrate, simulacao *sim);
void mostrar_atividades(atividade *primeira);
void mostrar_etapas(simulacao * s);
void pegar_produto_etapa(etapa *e, simulacao *s);
void pegar_produto_atividade(atividade *a, simulacao *sim);
int verificar_defeitos(produto *p, simulacao *s);
void avancar_produto(atividade *a, produto *p, simulacao *s);
void atualizar_atividade(atividade *a, simulacao *sim);
void envelhecer_atividade(atividade *a);
void liberar_fila(fila *f);
void liberar_atividade(atividade *a);
void liberar_atividades(etapa *e);
void liberar_etapas(etapas *e);
void liberar_pilha(pilha *p);
etapa *buscar_etapa(etapas *e, int id);
atividade *buscar_atividade(etapas *e, int id);
void contagem_regressiva( const char *mensagem);
void opcoes(simulacao *s);
int linha_vazia(simulacao *s);
int ha_produtos_para_entrar_etapa(etapa *e, simulacao *s);
int ha_produtos_para_entrar_atividade(atividade *a);
void envelhecer_produtos(simulacao *s);
void saidas(simulacao *s);
void entradas(simulacao *s);
void popular(simulacao *s);
void registrar_inicio_etapa(produto *p, simulacao *s);
void registrar_inicio_atividade(produto *p, simulacao *s);
int tempo_limite_excedido(simulacao *s);
noa * criar_noa(produto *p);
noa * inserir_noa(noa * raiz, produto *p);
noa * buscar_noa(noa * raiz, int id);
noa * remover_noa(noa * raiz, int id);
void liberar_arvore(noa ** raiz);
void liberar_historico(produto *p);
void liberar_produto(produto * p);
/*-----------------------
  FUNCÕES PUBLICAS
  ----------------------*/

void opcoes_finais(simulacao *s)
{   
    printf("\n\nSimulacao encerrada\n\n");
    
    inicializar_resumos(s);
    preencher_resumos(s->resumo, s->concluidos);

    while (1)
    {
        int opcao;
        
        printf("Opcoes:\n");
        printf("1 - Mostrar etapas e atividades\n");
        printf("2 - Mostrar fila\n");
        printf("3 - Mostrar pilha\n");
        printf("4 - Mostrar historico de produto\n");
        printf("5 - Imprimir relatorio\n");
        printf("6 - Sair do programa\n");
        
        scanf(" %d", &opcao);
        switch (opcao)
        {
        case 1:
        {
            mostrar_etapas(s);
        }
        break;
       
        case 2:
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
        
        case 3:
        {
            int op;
            printf("Pilha de concluidos ou lixo? (1 - concluidos, 2 - lixo): ");
            scanf("%d", &op);
            mostrar_pilha(op == 1 ? s->concluidos : s->lixo, s);
        }
        break;
        case 4:
        {
            
            int id;
            printf("Digite o id do produto que deseja visualizar: \n");
            scanf("%d", &id);
            noa * raiz = s->em_linha;
            raiz = buscar_noa(raiz, id);
            if(raiz){
                imprimir_historico_produto(s, raiz->p);
            }else{
                raiz = s->finalizados;
                raiz = buscar_noa(raiz, id); 
                    if(raiz)
                        imprimir_historico_produto(s, raiz->p);
                    else
                        printf("Produto nao encontrado\n");
            }

        }
        break;
        case 5:
        {
            int relatorio;
            printf("Mostrar qual relatorio?\n");
            printf("1 - Metadados\n");
            printf("2 - Relatorio de Etapas\n");
            printf("3 - Relatorio de Atividades\n");
            printf("4 - Relatorio de Produtos\n");
            printf("5 - Relatorio Completo(arquivo)\n");
            printf("6 - Voltar\n");
            scanf("%d", &relatorio);
            switch (relatorio)
            {
                case 1:
                {
                    imprimir_metadados(s);
                    break;  
                } 
                case 2:
                {
                    imprimir_relatorio_etapas(s);
                    break;
                }
                case 3:
                {
                    imprimir_relatorio_atividades(s);
                    break;
                }
                case 4:
                {
                    imprimir_relatorio_produtos(s);
                    break;
                }
                case 5:
                {   
                    relatorio_simulacao(s);
                    break;
                }
                case 6:
                {
                    break;
                }
                default:{
                    printf("Opcao invalida\n");
                    break;
                }
            }
            break;
        
        }
        case 6:
        {
            encerrar_simulacao(s);
        }
        exit(0);//redundante
        
        default:
        {
            printf("Opcao invalida\n");
            break;
        }
        
    }
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

    s->tick_atual = 0;
    while (!tempo_limite_excedido(s))
    {
        s->tick_atual++;
        printf("\nTick %d\n", s->tick_atual);
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
            Sleep(10); // periodo do clock em milisegundos, pode ser alterado depois
    }
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

void encerrar_simulacao(simulacao *s)
{
    printf("Liberando memoria\n");
    liberar_fila(s->fila_entrada);
    liberar_etapas(s->linha);
    liberar_pilha(s->lixo);
    liberar_pilha(s->concluidos);
    liberar_resumos(s);
    liberar_arvore(&(s->em_linha));
    liberar_arvore(&(s->finalizados));
    printf("Programa encerrado\n");
    exit(0);
}



/*-----------------------
  FUNCÕES AUXILIARES
  ----------------------*/

void opcoes(simulacao *s)
{
    while (1)
    {
        int opcao;
        printf("Opcoes:\n");
        printf("1 - Mostrar etapas e atividades\n");
        printf("2 - Mostrar fila\n");
        printf("3 - Mostrar pilha\n");
        printf("4 - Mostrar historico de produto\n");
        printf("5 - Continuar simulacao\n");
        printf("6 - Encerrar simulacao prematuramente\n");
        printf("7 - Alternar modo manual/automatico (atualmente %s)\n", s->MODO_MANUAL ? "manual" : "automatico");
        printf("8 - Sair do programa\n");
        
        scanf(" %d", &opcao);
        switch (opcao)
        {
        case 1:
        {
            mostrar_etapas(s);
        }
        break;
       
        case 2:
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
        
        case 3:
        {
            int op;
            printf("Pilha de concluidos ou lixo? (1 - concluidos, 2 - lixo): ");
            scanf("%d", &op);
            mostrar_pilha(op == 1 ? s->concluidos : s->lixo, s);
        }
        break;
        case 4:
        {
            int id;
            printf("Digite o id do produto que deseja visualizar: \n");
            scanf("%d", &id);
            noa * raiz = s->em_linha;
            raiz = buscar_noa(raiz, id);
            if(raiz){
                imprimir_historico_produto(s, raiz->p);
            }else{
                raiz = s->finalizados;
                raiz = buscar_noa(raiz, id); 
                    if(raiz)
                        imprimir_historico_produto(s, raiz->p);
                    else
                        printf("Produto nao encontrado\n");
            }

        }
        break;
        case 5:
        {
            contagem_regressiva("Continuando simulacao em");
            return;
        }
        break;

        case 6:
        {
            
            printf("Simulacao Encerrada\n");
            s->tempo_excedido = 1;
            return;
        }
        break;
        
        case 7:
        {
            s->MODO_MANUAL = !s->MODO_MANUAL;
            printf("Modo manual %s\n", s->MODO_MANUAL ? "ativado" : "desativado");
        }
        break;
        
        case 8:
        {
            encerrar_simulacao(s);
        }
        exit(0);//redundante
        
        default:
        {
            printf("Opcao invalida\n");
            break;
        }
        }
    }
}

noa * criar_noa(produto *p)
{
    noa * novo = malloc(sizeof(noa));
    novo->p = p;
    novo->esq = NULL;
    novo->dir = NULL;
    return novo;
}

noa * inserir_noa(noa *raiz, produto *p)
{
    if(!raiz)
    {
        raiz = criar_noa(p);
        return raiz;
    }

    if(p->id < raiz->p->id)
        raiz->esq = inserir_noa(raiz->esq, p);

    if(p->id > raiz->p->id)
        raiz->dir = inserir_noa(raiz->dir, p);

    return raiz;
}

noa * buscar_noa(noa * raiz, int id)
{
    if(!raiz || raiz->p->id == id)
        return raiz;
    
    if(id < raiz->p->id)
        return buscar_noa(raiz->esq, id); 

    return buscar_noa(raiz->dir, id); 
}

noa * remover_noa(noa * raiz, int id)
{
    if(!raiz)
    {   
        printf("Produto nao se encontra na arvore\n");
        return NULL;
    }

    if(id < raiz->p->id){
        raiz->esq = remover_noa(raiz->esq, id);
        return raiz;
    }
    if(id > raiz->p->id){
        raiz->dir = remover_noa(raiz->dir, id);
        return raiz;
    }

    if(!raiz->esq && !raiz->dir){
        free(raiz);
        return NULL;
    }else if(raiz->esq && raiz->dir){
        noa * p_aux = raiz;
        noa * aux = raiz->esq;
        while(aux->dir){
            p_aux = aux;
            aux = aux->dir;
        }
        if(p_aux != raiz){
            p_aux->dir = aux->esq;
            aux->esq = raiz->esq;
        }
        aux->dir = raiz->dir;
        free (raiz);
        return aux;

    }else{
        noa * aux = raiz->dir? raiz->dir : raiz->esq;
        free(raiz);
        return aux;
    }
       
}

void liberar_arvore(noa ** raiz)
{   
    if(*raiz)
    {
        liberar_arvore(&((*raiz)->esq));
        liberar_arvore(&((*raiz)->dir));
        free(*raiz);
        *raiz = NULL;
    }
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
void empilhar(produto *p, pilha **pp, simulacao * s)
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
        s->em_linha = remover_noa(s->em_linha, p->id);
        s->finalizados = inserir_noa(s->finalizados, p);
        return;
    }
    p->proximo_produto = (*pp)->topo;
    (*pp)->topo = p;
    s->em_linha = remover_noa(s->em_linha, p->id);
    s->finalizados = inserir_noa(s->finalizados, p);
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
    enfileirar(novo, &s->fila_entrada);
    novo->id = s->produto_id;
    novo->defeituoso = 0;
    novo->falhas = 0;


    novo->tick_criacao = 0;
    novo->tick_entrada_linha= 0;
    novo->tick_saida_linha= 0;


    novo->historico_etapas = NULL;
    novo->evento_atual_etapa = NULL;
    
    novo->tick_criacao = s->tick_atual;

    s->produto_id++;
    s->produtos_criados++;
    s->em_linha = inserir_noa(s->em_linha, novo);
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

void falhar_produto(produto *p, float failrate, simulacao *s)
{
    float r = (float)rand() / RAND_MAX; // gera um número aleatório entre 0 e 1
    if (r < failrate)
    {   
        p->evento_atual_etapa->falhou = 1;
        p->evento_atual_etapa->evento_atual_atividade->falhou = 1;
        p->falhas++;
        p->etapa_atual->falhas++;
        s->falhas_totais++;
        if (r < failrate / 2 || p->falhas > 3)
        { // se for uma falha catastrófica ou se o produto já tiver falhado mais de 3 vezes, considera-se o produto como defeituoso e não tenta consertar mais
            printf("Produto %d teve uma falha catastrófica na atividade %d\n", p->id, p->atividade_atual->id);
            p->evento_atual_etapa->falhou = 2;
            p->evento_atual_etapa->evento_atual_atividade->falhou = 2;
            p->defeituoso = 2;
            return;
        }
        printf("Produto %d falhou na atividade %d\n", p->id, p->atividade_atual->id);
        p->defeituoso = 1;
        return;
    }
    printf("Produto %d processado com sucesso na atividade %d\n", p->id, p->atividade_atual->id);
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
void mostrar_etapas(simulacao *s)
{
    if (!s->linha || !s->linha->primeira_etapa) 
    {
        printf("Nenhuma etapa criada\n");
        return;
    }
    if(s->fila_entrada)
        printf("Fila de entrada: %d produtos\n", s->fila_entrada->em_fila);
    etapa *atual = s->linha->primeira_etapa;
    while (atual != NULL)
    {
        printf("%s\n", atual->nome);
        mostrar_atividades(atual->primeira_atividade);
        printf("Fila de prontos da etapa %d:\n", atual->id);
        mostrar_fila(atual->f);
        atual = atual->proxima_etapa;
    }
}

void pegar_produto_etapa(etapa *e, simulacao *s)
{ //só é chamada quando há produto pra ser pego
    
    produto *p;
    if (e->etapa_anterior)
    {
        printf("Pegando produto da fila de prontos da etapa anterior\n");
        p = desenfileirar(e->etapa_anterior->f); // pega o primeiro produto da fila de prontos da etapa anterior
        p->evento_atual_etapa->tick_fim = s->tick_atual;
        if (p)
        {
            e->etapa_anterior->ocupacao--;
        }
    }
    else
    {
        printf("Primeira etapa, pegando produto da fila de entrada\n");
        p = desenfileirar(s->fila_entrada); // pega o primeiro produto da fila de entrada
        p->tick_entrada_linha = s->tick_atual;//não segfaulta pq a função só é chamada se tiver produto pra entrar
    }
    if (!p)//verificação redundante 
    {
        printf("Nenhum produto disponível para pegar na etapa %d\n", e->id);
        return;
    }
    atividade *a = e->primeira_atividade;
    enfileirar(p, &a->f); // coloca o produto na fila de entrada da primeira atividade da etapa
    p->atividade_atual = a;
    p->etapa_atual = e;
    e->ocupacao++;
    e->qtd_produtos_entraram++;
    printf("Produto %d entrou na fila de entrada da Atividade %d\n", p->id, a->id);
    registrar_inicio_etapa(p, s);
}
void pegar_produto_atividade(atividade *a, simulacao *sim)
{//só é chamada quando há produto pra ser pego
    
    produto *p = desenfileirar(a->f); // pega o primeiro produto da fila de entrada da atividade
    for (int i = 0; i < a->capacidade_max; i++)
    {
        if (!a->slots[i].p)//bota o produto no primeiro slot vazio
        {
            a->slots[i].p = p;
            a->slots[i].tempo_restante = a->tempo_de_processamento; // tempo de processamento da atividade, pode ser alterado depois
            a->ocupacao++;
            p->atividade_atual = a;
            p->evento_atual_etapa->evento_atual_atividade->tick_inicio_processamento = sim->tick_atual;
            printf("Produto %d saiu da fila de entrada e entrou no slot %d da Atividade %d\n", p->id, i, a->id);
            break;
            
        };
    }
}
int verificar_defeitos(produto *p, simulacao *s)
{
    if (p->defeituoso)
    {
        etapa *e = p->atividade_atual->etapa_dona;
        if (p->defeituoso == 2)
        {
            printf("Produto %d processado com falha catastrofica e direcionado a pilha de lixo\n", p->id);
            empilhar(p, &s->lixo, s);
            p->tick_saida_linha = -1 * (s->tick_atual);
            p->evento_atual_etapa->tick_fim = -1 * (s->tick_atual);
            e->ocupacao--;
            return 1;
        }
        printf("Produto %d processado com falha recuperável, redirecionado para o começo da etapa %d\n", p->id, p->atividade_atual->etapa_dona->id);
        p->defeituoso = 0;
        enfileirar(p, &(e->primeira_atividade->f)); 
        p->atividade_atual = e->primeira_atividade;
        registrar_inicio_etapa(p, s);
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
        enfileirar(p, &a->proxima_atividade->f); // coloca o produto na fila de entrada da proxima atividade
        printf("Produto %d colocado na fila de entrada da proxima atividade (%d)\n", p->id, a->proxima_atividade->id);
        p->atividade_atual = a->proxima_atividade;
        registrar_inicio_atividade(p, s);
        return;
    }
    printf("Produto %d concluiu a ultima atividade da etapa %d\n", p->id, a->etapa_dona->id);
    p->etapa_atual->qtd_produtos_concluidos++;
    p->evento_atual_etapa->tick_conclusao = s->tick_atual;//vai pra fila de prontos ou pra pilha de concluido
    if (a->etapa_dona->proxima_etapa)
    {
        printf("Enfileirando produto %d na fila de prontos da etapa\n", p->id);
        enfileirar(p, &a->etapa_dona->f);
        return;
    }
    printf("Produto %d concluiu a ultima etapa do processo\n", p->id);
    p->evento_atual_etapa->tick_fim = s->tick_atual;
    empilhar(p, &s->concluidos, s);
    s->produtos_concluidos++;
    p->etapa_atual = NULL;
    p->atividade_atual = NULL;
    p->tick_saida_linha = s->tick_atual;
    a->etapa_dona->ocupacao--;
    return;
}
void atualizar_atividade(atividade *a, simulacao *sim)
{ // atualiza todos os produtos da atividade

    for (int i = 0; i < a->capacidade_max; i++)
    { // olha cada slot
        slot *s = &a->slots[i];
        if (s->p)
        { // se tiver produto no slot
            if (s->tempo_restante <= 0) // se o produto terminou de ser processado
            {
                produto *p = s->p;
                p->evento_atual_etapa->evento_atual_atividade->tick_fim_processamento = sim->tick_atual;
                printf("Slot %d: Produto %d terminou de ser processado na Atividade %d\n", i, p->id, a->id);
                falhar_produto(p, a->failrate, sim); // joga os dados para falhar o produto
                if(!a->proxima_atividade){
                    falhar_produto(p, a->etapa_dona->failrate, sim);
                }
                avancar_produto(a, p, sim);
                a->slots[i].p = NULL; 
            }
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
        liberar_produto(aux);
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
            liberar_produto(a->slots[i].p);
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
        liberar_produto(aux);
    }
    free(p);
}
void liberar_produto(produto * p)
{
    if(!p)
        return;
    liberar_historico(p);
    free(p);
}
void liberar_historico(produto *p)
{
    if(!p->historico_etapas)
        return;
    evento_etapa * atual = p->historico_etapas;
    while(atual)
    {
        if(atual->historico_atividades)
        {
            evento_atividade *a_atual = atual->historico_atividades;
            while(a_atual)
            {
                evento_atividade *aux = a_atual;
                a_atual = a_atual->proximo_evento;
                free(aux);
            }
        }
        evento_etapa *aux = atual;
        atual = atual->proximo_evento;
        free(aux);
    }
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
            pegar_produto_etapa(atual_etapa, s);
        }
        atividade *atual_atividade = atual_etapa->primeira_atividade;
        while (atual_atividade) // para cada atividade da etapa (qualquer ordem):
        {
            while (atual_atividade->ocupacao < atual_atividade->capacidade_max && ha_produtos_para_entrar_atividade(atual_atividade))
            {   
                pegar_produto_atividade(atual_atividade, s);
                
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

void registrar_inicio_etapa(produto *p, simulacao *s)
{
    evento_etapa *evento = malloc(sizeof(evento_etapa));
    if (!evento)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    evento->e = p->etapa_atual;
    evento->tick_inicio = s->tick_atual;
    evento->falhou = 0;
    evento->tentativa = 1;
    evento->proximo_evento = NULL;
    
    evento->tick_conclusao = -1;
    evento->tick_fim = -1; //valores negativos só pra poder perceber quando não forem atualizado
    evento->historico_atividades = NULL;
    evento->evento_atual_atividade = NULL;
    
    //inserir na lista e atualizar ponteiros do produto
    if (!p->historico_etapas)
    {
        p->historico_etapas = evento;
        p->evento_atual_etapa = evento;
    }
    else
    {
        if(p->evento_atual_etapa->falhou)//como evento_atual_etapa não foi atualizado, ainda aponta pro evento_etapa anterior/último.
            evento->tentativa = p->evento_atual_etapa->tentativa + 1;
        p->evento_atual_etapa->proximo_evento = evento;
        p->evento_atual_etapa = evento;
    }

    registrar_inicio_atividade(p, s);
  
}
void registrar_inicio_atividade(produto *p, simulacao *s)
{
    evento_atividade *evento = malloc(sizeof(evento_atividade));
    if (!evento)
    {
        printf("\n\nErro ao alocar memória! \n\n");
        exit(1);
    }
    evento->a = p->atividade_atual;
    evento->falhou = 0;
    evento->tick_fila = s->tick_atual;
    evento->tick_inicio_processamento = -1;
    evento->tick_fim_processamento = -1;

    evento->proximo_evento = NULL;

    //inserir na lista e atualizar ponteiros do produto
    evento_etapa *atual_etapa = p->evento_atual_etapa;
    if (!atual_etapa->historico_atividades)
    {
      atual_etapa->historico_atividades = evento;
        atual_etapa->evento_atual_atividade = evento;
    }
    else
    {
        atual_etapa->evento_atual_atividade->proximo_evento = evento;
        atual_etapa->evento_atual_atividade = evento;
    }
}

int tempo_limite_excedido(simulacao *s)
{
    if(s->tick_atual >= s->max_ticks)
        s->tempo_excedido = 1;
    return s->tempo_excedido;
}