#include <stdio.h>
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

    criar_etapa(&s);
    criar_etapa(&s);
    criar_etapa(&s);

    simular(&s);//atentar para o tick maximo

    mostrar_pilha(s.concluidos, &s);
    mostrar_pilha(s.lixo, &s);
    mostrar_relatorio_etapas(s.linha, s.concluidos);
    
    encerrar_simulacao(&s);
}