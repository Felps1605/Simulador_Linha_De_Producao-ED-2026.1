#include <stdio.h>
#include "structs.h"
#include "construcao.h"
#include "execucao.h"

int main()
{
    //srand(time(NULL));
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