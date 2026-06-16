#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "entrada.h"
#include "execucao.h"
#include "output.h"


int main()
{
    simulacao s;
    
    FILE *arquivo = fopen("C:\\Users\\felps\\Desktop\\ED_AJALMAR\\entrada.txt", "r");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    lerEntrada(arquivo, &s);

    fclose(arquivo);
    
    srand(s.semente);
    
    printf("Modo manual(1) ou automatico(0)?\n");
    scanf("%d", &s.MODO_MANUAL);

    simular(&s);//atentar para o tick maximo

    mostrar_pilha(s.concluidos, &s);
    mostrar_pilha(s.lixo, &s);

    opcoes_finais(&s);

    encerrar_simulacao(&s);
}