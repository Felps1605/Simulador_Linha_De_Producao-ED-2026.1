#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "output.h"


//preciso de algum tipo de lista com todos os produtos
//por enquanto vou testar só com um produto mesmo.


void mostrar_relatorio_etapas(etapas *e, pilha *p)
{   
    if(!e)
    {
        printf("linha invalido!\n");
    }
    
    //em cada evento de etapas eu mostro os eventos de atividade 
    etapa * atual = e->primeira_etapa;
    printf("\n--------------RELATORIO DE ETAPAS--------------\n");
    while(atual)
    {   
        printf("ETAPA   %d, %s: \n", atual->id, atual->nome);
        printf("\tAtividades: %d \n", atual->num_atividades);
        printf("\tFalhas Totais: %d \n", atual->falhas);
        printf("\tQuantide de produtos que entraram: %d \n", atual->qtd_produtos_entraram);
        printf("\tQuantidade de produtos concluidos: %d \n", atual->qtd_produtos_concluidos);
        printf("\tMedia de falhas por produto: %.2f \n", ((float)atual->falhas / atual->qtd_produtos_entraram));
        printf("\tTempo minimo: %d \n", tempo_minimo(atual));
        printf("\tTempo medio: %.2f \n", tempo_medio_produtos_finalizados(atual, p));
        printf("\tMaior tempo: %d \n", tempo_maximo_produtos_finalizados(atual, p));
        printf("\tTempo medio total em filas de atviidade: %.2f \n", tempo_medio_em_fila_finalizados(atual, p).tempo_em_fila_de_atividades);
        printf("\tTempo medio da fila de prontos da atividade: %.2f \n", tempo_medio_em_fila_finalizados(atual, p).tempo_em_fila_de_prontos);
        atual = atual->proxima_etapa;
    }
    
}

int tempo_minimo(etapa *e)
{
    if(!e)
    {
        return -1;
    }
    int soma = 0;
    atividade * atual = e->primeira_atividade;
    while(atual)
    {
        soma = soma + atual->tempo_de_processamento;
        atual = atual->proxima_atividade;
    }
    return soma;
}
float tempo_medio_produtos_finalizados(etapa *e, pilha *p)//funcao arcaica que calcula com base na pilha de concluidos
{//posteriormente precisa ser ajustado para algu que calcule com base em todos os produtos que passaram por aquela etapa específica.
    float soma = 0;
    produto *atual = p->topo;
    while(atual)
    {   
        //percorrer_historico_etapas de cada produto?não, por enquanto essa função é por etapa
        //cada iteração teria que varrer a lista de eventos até chegar na etapa certa?
        //por enquanto sim fodase
        int tempo = buscar_tempo_na_etapa(atual, e->id);
        if (tempo < 0)
        {
            //printf("Ocorreu um erro no calculo de tempo medio (Produto %d)", atual->id);
            return -1;
        }
       
        soma = soma + tempo;
        atual = atual->proximo_produto;
    }
    return (soma / p->em_pilha);
}
int tempo_maximo_produtos_finalizados(etapa *e, pilha *p)
{   
    float maior = -1;
    produto *atual = p->topo;
    while(atual)
    {   
        
        int tempo = buscar_tempo_na_etapa(atual, e->id);
        if (tempo < 0)
        {
            printf("Ocorreu um erro no calculo de maior (Produto %d)", atual->id);
            return -1;
        }
        if(tempo > maior)
            maior = tempo;
        atual = atual->proximo_produto;
    }
    return (maior);

}


tempos tempo_medio_em_fila_finalizados(etapa *e, pilha * p)
{ //posteriormente precisa ser ajustado para algo que calcule com base em todos os produtos que passaram por aquela etapa específica.
    tempos soma;
    soma.tempo_em_fila_de_atividades = 0;
    soma.tempo_em_fila_de_prontos = 0;
    produto *atual = p->topo;
    while(atual)
    {   
        //para cada produto na pilha de concluidos 
        tempos t;//vai conter os tempos que o produto passou nas filas de atividades e de prontos
        setar_tempo_em_fila_na_etapa(&t, atual, e->id);//tempo de fila de cada atividade + fila de prontos
        atual = atual->proximo_produto;
        soma.tempo_em_fila_de_atividades = soma.tempo_em_fila_de_atividades +t.tempo_em_fila_de_atividades;
        soma.tempo_em_fila_de_prontos = soma.tempo_em_fila_de_prontos + t.tempo_em_fila_de_prontos;
    }
    soma.tempo_em_fila_de_atividades = (soma.tempo_em_fila_de_atividades /p->em_pilha);
    soma.tempo_em_fila_de_prontos = (soma.tempo_em_fila_de_prontos /p->em_pilha);
    return soma;
}

void setar_tempo_em_fila_na_etapa(tempos * t, produto * p, int id)
{
    
    float somat1 = 0, somat2 = 0;
    evento_etapa * atual = p->historico_etapas;
    while(atual)
    {
        if(atual->e->id == id){
            
            while(atual->proximo_evento && atual->proximo_evento->e->id == id)//proteção pra não dar segfault quando chegar no evento da ultima etapa
            {
                somat1 = somat1 + buscar_tempo_em_fila_nas_atividades(atual);
                somat2 = somat2 + (atual->tick_conclusao - atual->tick_fim);
                atual = atual->proximo_evento;
            }//quando o loop acaba o ponteiro ta apontando para ultimo evento daquela etapa
            t->tempo_em_fila_de_atividades = (somat1 / atual->tentativa);
            t->tempo_em_fila_de_prontos = (somat2 / atual->tentativa);
        }
        atual = atual->proximo_evento;
    }
    

}

int buscar_tempo_na_etapa(produto * p, int id)
{
    if(!p || !p->historico_etapas)
    {
        printf("Produto invalido\n");
    }
    int tick_inicio;
    int tick_fim;
    evento_etapa * atual = p->historico_etapas;
    while(atual)
    {
        if(atual->e->id == id){
            tick_inicio = atual->tick_inicio;
            while(atual->proximo_evento && atual->proximo_evento->e->id == id)//proteção pra não dar segfault quando chegar no evento da ultima etapa
            {
                atual = atual->proximo_evento;
            }//quando o loop acaba o ponteiro ta apontando para ultimo evento daquela etapa
            tick_fim = atual->tick_fim;
            return (tick_fim-tick_inicio);
        }
        atual = atual->proximo_evento;
    }
    printf("Produto %d não tem Etapa %d registrada no seu historico de eventos\n", p->id, id );
    return -1;

}

int buscar_tempo_em_fila_nas_atividades(evento_etapa *e)
{
    int soma = 0;
    evento_atividade * atual = e->historico_atividades;
    while(atual)
    {   
        //para cada atividade calcula o tempo em fila e soma
        soma = soma + (atual->tick_inicio_processamento - atual->tick_fila);
        atual = atual->proximo_evento;
    }
    return soma;
}