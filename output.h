#ifndef OUTPUT_H
#define OUTPUT_H

#include "structs.h"

typedef struct tempos{
    float tempo_em_fila_de_atividades;
    float tempo_em_fila_de_prontos;
}tempos;

void mostrar_metadados(simulacao *s);
void mostrar_relatorio_etapas(simulacao *s);
void mostrar_relatorio_atividades(simulacao *s);
void mostrar_trajetoria_produtos(simulacao *s);

#endif