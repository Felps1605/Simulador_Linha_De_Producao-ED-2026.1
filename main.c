#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "construcao.h"
#include "execucao.h"
#include "output.h"

int main()
{
    simulacao s;
    inicializar_simulacao(&s);
    srand(s.semente);
    printf("testar relatorio e funcionamento normal\n");
    printf("Modo manual(1) ou automatico(0)?\n");
    scanf("%d", &s.MODO_MANUAL);

    for(int i = 0 ; i < s.n_etapas; i ++)
    {
        criar_etapa(&s);
    }

    simular(&s);//atentar para o tick maximo

    mostrar_pilha(s.concluidos, &s);
    mostrar_pilha(s.lixo, &s);

    mostrar_metadados(&s);
    mostrar_relatorio_etapas(&s);
    mostrar_relatorio_atividades(&s);
    
    encerrar_simulacao(&s);
}