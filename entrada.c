/* Bibliotecas padrão necessárias para entrada/saída, alocação e strings */
#include <stdio.h>   /* printf, fprintf, fopen, fclose, fgets, sscanf  */
#include <stdlib.h>  /* EXIT_SUCCESS, EXIT_FAILURE                      */
#include <string.h>  /* memset, memmove, strlen, strspn, strncmp, strcpy */

/* ══════════════════════════════════════════════════════════════════
   ESTRUTURAS DE DADOS
   Cada struct representa exatamente um bloco do arquivo de template.
   ══════════════════════════════════════════════════════════════════ */

/* Representa uma ATIVIDADE: menor unidade de trabalho dentro de uma etapa */
typedef struct {
    int    id;              /* Identificador numérico da atividade              */
    double tempo_finalizar; /* Tempo (em segundos) necessário para concluí-la  */
    double taxa_falha;      /* Probabilidade de falha durante a execução        */
    char   nome[64];        /* Nome descritivo opcional (até 63 chars + '\0')   */
} Atividade;

/* Representa uma ETAPA: agrupa atividades e define capacidade/falha do estágio */
typedef struct {
    int       id;                    /* Identificador numérico da etapa                     */
    int       quantidade_atividades; /* Número de atividades declaradas no template         */
    int       capacidade_total;      /* Quantas unidades a etapa pode processar ao mesmo tempo */
    double    taxa_falha_inicial;    /* Taxa de falha logo ao iniciar a etapa               */
    char      nome[64];              /* Nome descritivo opcional da etapa                   */
    Atividade atividades[64];        /* Array fixo para armazenar até 64 atividades         */
    int       num_atividades_lidas;  /* Contador de atividades efetivamente lidas do arquivo */
} Etapa;

/* Representa o cabeçalho SIMULACAO: parâmetros globais da simulação */
typedef struct {
    char   nome[64];         /* Nome identificador da simulação               */
    int    semente;          /* Semente para o gerador de números aleatórios  */
    double limite_tempo_seg; /* Tempo máximo (seg) que a simulação pode durar */
} Simulacao;

/* Representa o bloco PRODUTO: o que será fabricado na linha */
typedef struct {
    int    quantidade;  /* Total de unidades a produzir              */
    double taxa_por_seg; /* Velocidade de produção em unidades/segundo */
    char   nome[64];    /* Nome do produto                           */
} Produto;

/* Representa a LINHA_PRODUCAO: contém todas as etapas em sequência */
typedef struct {
    int   quantidade_etapas; /* Número de etapas declaradas no template        */
    Etapa etapas[32];        /* Array fixo para armazenar até 32 etapas        */
    int   num_etapas_lidas;  /* Contador de etapas efetivamente lidas do arquivo */
} LinhaProducao;

/* Estrutura raiz que agrupa todos os dados do arquivo de configuração */
typedef struct {
    Simulacao    simulacao; /* Dados do bloco SIMULACAO      */
    Produto      produto;   /* Dados do bloco PRODUTO        */
    LinhaProducao linha;    /* Dados do bloco LINHA_PRODUCAO */
} SimuladorCompleto;

/* ══════════════════════════════════════════════════════════════════
   FUNÇÃO: trim
   Remove espaços, tabs e quebras de linha do início e do fim de 's'.
   Necessário para que strncmp funcione corretamente nos tokens lidos.
   ══════════════════════════════════════════════════════════════════ */
static void trim(char *s) {
    char *p = s + strspn(s, " \t\r\n"); /* Avança 'p' até o primeiro char não-branco     */
    memmove(s, p, strlen(p) + 1);       /* Desloca o conteúdo para o início do buffer    */
    size_t len = strlen(s);             /* Recalcula o comprimento após mover             */
    /* Remove caracteres brancos do final, sobrescrevendo com '\0' */
    while (len > 0 && (s[len-1] == ' '  ||
                       s[len-1] == '\t' ||
                       s[len-1] == '\r' ||
                       s[len-1] == '\n'))
        s[--len] = '\0'; /* Decrementa len e apaga o último caractere branco */
}

/* ══════════════════════════════════════════════════════════════════
   FUNÇÃO: ler_arquivo
   Abre e interpreta o arquivo de template linha a linha,
   preenchendo a struct SimuladorCompleto apontada por 'sim'.
   Retorna  0 em caso de sucesso.
   Retorna -1 em caso de erro (arquivo inválido ou formato incorreto).
   ══════════════════════════════════════════════════════════════════ */
int ler_arquivo(const char *caminho, SimuladorCompleto *sim) {
    FILE *f = fopen(caminho, "r"); /* Abre o arquivo somente para leitura ("r") */
    if (!f) {                      /* Se fopen retornou NULL o arquivo não existe ou sem permissão */
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", caminho);
        return -1; /* Encerra indicando falha */
    }

    memset(sim, 0, sizeof(*sim)); /* Zera toda a struct antes de popular, evitando lixo de memória */

    char linha[512]; /* Buffer que receberá cada linha do arquivo (até 511 chars + '\0') */
    int  etapa_atual = -1; /* Índice da última ETAPA lida; -1 = nenhuma etapa lida ainda  */
    int  num_linha   = 0;  /* Contador de linhas para mensagens de erro precisas          */

    /* Loop principal: lê uma linha por iteração até EOF */
    while (fgets(linha, sizeof(linha), f)) {
        num_linha++;    /* Incrementa o contador a cada linha lida          */
        trim(linha);    /* Remove espaços/quebras antes de comparar o token */

        /* Pula linhas em branco (trim deixou string vazia) e comentários com '#' */
        if (linha[0] == '\0' || linha[0] == '#')
            continue; /* Volta para o início do while sem processar esta linha */

        /* ── Bloco SIMULACAO ────────────────────────────────────────────────
           Formato esperado: SIMULACAO <nome> <semente> <limite_seg>         */
        if (strncmp(linha, "SIMULACAO", 9) == 0) {
            /* sscanf retorna o número de campos convertidos com sucesso      */
            /* %63s lê no máximo 63 chars (deixa espaço para '\0') no nome   */
            int n = sscanf(linha, "SIMULACAO %63s %d %lf",
                           sim->simulacao.nome,          /* destino: nome da simulação    */
                           &sim->simulacao.semente,      /* destino: semente aleatória    */
                           &sim->simulacao.limite_tempo_seg); /* destino: limite em seg   */
            if (n < 3) { /* Menos de 3 campos lidos = linha malformada */
                fprintf(stderr, "Linha %d: formato SIMULACAO invalido\n", num_linha);
                fclose(f); /* Fecha o arquivo antes de retornar erro */
                return -1;
            }
        }

        /* ── Bloco PRODUTO ──────────────────────────────────────────────────
           Formato esperado: PRODUTO <quantidade> <taxa_por_seg> <nome>      */
        else if (strncmp(linha, "PRODUTO", 7) == 0) {
            int n = sscanf(linha, "PRODUTO %d %lf %63s",
                           &sim->produto.quantidade,  /* destino: total a produzir       */
                           &sim->produto.taxa_por_seg, /* destino: velocidade de produção */
                           sim->produto.nome);         /* destino: nome do produto        */
            if (n < 3) { /* Linha incompleta ou com tipos errados */
                fprintf(stderr, "Linha %d: formato PRODUTO invalido\n", num_linha);
                fclose(f);
                return -1;
            }
        }

        /* ── Bloco LINHA_PRODUCAO ───────────────────────────────────────────
           Formato esperado: LINHA_PRODUCAO <quantidade_etapas>              */
        else if (strncmp(linha, "LINHA_PRODUCAO", 14) == 0) {
            int n = sscanf(linha, "LINHA_PRODUCAO %d",
                           &sim->linha.quantidade_etapas); /* destino: número de etapas */
            if (n < 1) { /* Campo numérico ausente */
                fprintf(stderr, "Linha %d: formato LINHA_PRODUCAO invalido\n", num_linha);
                fclose(f);
                return -1;
            }
            /* Verifica se o número declarado cabe no array estático de 32 posições */
            if (sim->linha.quantidade_etapas > 32) {
                fprintf(stderr, "Linha %d: numero de etapas excede o limite (32)\n", num_linha);
                fclose(f);
                return -1;
            }
        }

        /* ── Bloco ETAPA ────────────────────────────────────────────────────
           Formato esperado: ETAPA <id> <qtd_ativ> <capacidade> <taxa_falha> [nome]
           O campo [nome] é opcional; se ausente, usa "(sem nome)"           */
        else if (strncmp(linha, "ETAPA", 5) == 0) {
            Etapa e;              /* Variável local temporária para a etapa lida    */
            memset(&e, 0, sizeof(e)); /* Zera todos os campos antes de popular     */

            /* %63[^\n] lê até 63 chars ou até encontrar '\n', capturando o nome
               que pode conter espaços — diferente de %s que para no espaço       */
            int n = sscanf(linha, "ETAPA %d %d %d %lf %63[^\n]",
                           &e.id,                    /* identificador da etapa      */
                           &e.quantidade_atividades, /* qtd declarada no template   */
                           &e.capacidade_total,      /* capacidade de processamento */
                           &e.taxa_falha_inicial,    /* taxa de falha ao iniciar    */
                           e.nome);                  /* nome opcional               */
            if (n < 4) { /* Os 4 primeiros campos são obrigatórios */
                fprintf(stderr, "Linha %d: formato ETAPA invalido\n", num_linha);
                fclose(f);
                return -1;
            }
            if (n == 4) strcpy(e.nome, "(sem nome)"); /* Nome ausente: usa padrão  */
            trim(e.nome); /* Remove possíveis espaços extras ao redor do nome       */

            /* Verifica se ainda há espaço no array antes de inserir */
            if (sim->linha.num_etapas_lidas >= 32) {
                fprintf(stderr, "Linha %d: numero de etapas excede o limite (32)\n", num_linha);
                fclose(f);
                return -1;
            }
            etapa_atual = sim->linha.num_etapas_lidas; /* Marca esta posição como "etapa atual" */
            sim->linha.etapas[etapa_atual] = e;        /* Copia a struct local para o array     */
            sim->linha.num_etapas_lidas++;             /* Avança o contador de etapas           */
        }

        /* ── Bloco ATIVIDADE ────────────────────────────────────────────────
           Formato esperado: ATIVIDADE <id> <tempo> <taxa_falha> [nome]
           Pertence à última ETAPA lida (controlado por etapa_atual)         */
        else if (strncmp(linha, "ATIVIDADE", 9) == 0) {
            /* Garante que existe ao menos uma etapa pai antes de inserir */
            if (etapa_atual < 0) {
                fprintf(stderr, "Linha %d: ATIVIDADE sem ETAPA pai\n", num_linha);
                fclose(f);
                return -1;
            }
            Etapa *ep = &sim->linha.etapas[etapa_atual]; /* Ponteiro para a etapa atual       */
            /* Verifica se o array de atividades desta etapa ainda tem espaço */
            if (ep->num_atividades_lidas >= 64) {
                fprintf(stderr, "Linha %d: numero de atividades excede o limite (64)\n", num_linha);
                fclose(f);
                return -1;
            }

            Atividade a;              /* Variável local temporária para a atividade  */
            memset(&a, 0, sizeof(a)); /* Zera todos os campos antes de popular       */

            int n = sscanf(linha, "ATIVIDADE %d %lf %lf %63[^\n]",
                           &a.id,             /* identificador da atividade           */
                           &a.tempo_finalizar, /* tempo necessário para concluir (seg) */
                           &a.taxa_falha,      /* probabilidade de falha durante exec  */
                           a.nome);            /* nome opcional                        */
            if (n < 3) { /* Os 3 primeiros campos são obrigatórios */
                fprintf(stderr, "Linha %d: formato ATIVIDADE invalido\n", num_linha);
                fclose(f);
                return -1;
            }
            if (n == 3) strcpy(a.nome, "(sem nome)"); /* Nome ausente: usa padrão    */
            trim(a.nome); /* Remove possíveis espaços extras ao redor do nome         */

            /* Insere a atividade no array da etapa pai e incrementa o contador */
            ep->atividades[ep->num_atividades_lidas++] = a;
        }

        /* ── Linha de título do arquivo ─────────────────────────────────────
           "SIMULADOR DE LINHA DE PRODUCAO" é apenas um cabeçalho visual,
           não carrega dados — simplesmente ignoramos                         */
        else if (strncmp(linha, "SIMULADOR DE LINHA", 18) == 0) {
            /* Nenhuma ação necessária: linha de título informativa */
        }

        /* ── Token desconhecido ─────────────────────────────────────────────
           Qualquer linha que não se encaixe nos padrões acima gera um aviso
           mas NÃO interrompe a leitura (comportamento tolerante a erros)    */
        else {
            fprintf(stderr, "Aviso linha %d: token desconhecido ignorado: '%s'\n",
                    num_linha, linha);
        }
    } /* fim do while(fgets) */

    fclose(f); /* Fecha o arquivo após ler todas as linhas com sucesso */
    return 0;  /* Retorna 0 indicando que a leitura foi bem-sucedida   */
}

/* ══════════════════════════════════════════════════════════════════
   FUNÇÃO: imprimir_simulador
   Exibe no terminal todos os dados armazenados em 'sim',
   usando formatação hierárquica para facilitar a leitura.
   ══════════════════════════════════════════════════════════════════ */
void imprimir_simulador(const SimuladorCompleto *sim) {
    /* Cabeçalho visual do relatório */
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║        SIMULADOR DE LINHA DE PRODUCAO            ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    /* Seção SIMULACAO: exibe os três campos do bloco */
    printf("── SIMULACAO ──────────────────────────────────────\n");
    printf("  Nome             : %s\n",    sim->simulacao.nome);            /* Nome da simulação     */
    printf("  Semente          : %d\n",    sim->simulacao.semente);         /* Semente aleatória     */
    printf("  Limite (seg)     : %.2f\n\n",sim->simulacao.limite_tempo_seg); /* Limite em segundos   */

    /* Seção PRODUTO: exibe os três campos do bloco */
    printf("── PRODUTO ────────────────────────────────────────\n");
    printf("  Nome             : %s\n",    sim->produto.nome);       /* Nome do produto            */
    printf("  Quantidade       : %d\n",    sim->produto.quantidade); /* Total a produzir            */
    printf("  Taxa (prod/seg)  : %.4f\n\n",sim->produto.taxa_por_seg); /* Velocidade de produção   */

    /* Seção LINHA DE PRODUCAO: exibe o número de etapas lidas */
    printf("── LINHA DE PRODUCAO (%d etapa(s)) ────────────────\n",
           sim->linha.num_etapas_lidas);

    /* Itera sobre cada etapa armazenada */
    for (int i = 0; i < sim->linha.num_etapas_lidas; i++) {
        const Etapa *e = &sim->linha.etapas[i]; /* Ponteiro de conveniência para a etapa i */

        /* Exibe os campos da etapa com recuo visual */
        printf("\n  ┌─ ETAPA %d  \"%s\"\n",      e->id, e->nome);
        printf("  │  Atividades declaradas : %d\n", e->quantidade_atividades);
        printf("  │  Capacidade total      : %d\n", e->capacidade_total);
        printf("  │  Taxa de falha inicial : %.4f\n",e->taxa_falha_inicial);
        printf("  │  Atividades lidas      : %d\n", e->num_atividades_lidas);

        /* Itera sobre cada atividade da etapa corrente */
        for (int j = 0; j < e->num_atividades_lidas; j++) {
            const Atividade *a = &e->atividades[j]; /* Ponteiro de conveniência para atividade j */

            /* Exibe os campos da atividade com recuo adicional */
            printf("  │    ├─ ATIVIDADE %d  \"%s\"\n",   a->id, a->nome);
            printf("  │    │  Tempo p/ finalizar : %.4f seg\n", a->tempo_finalizar);
            printf("  │    │  Taxa de falha      : %.4f\n",     a->taxa_falha);
        }
    }
    printf("\n"); /* Linha em branco ao final para separar do prompt do terminal */
}
/* ══════════════════════════════════════════════════════════════════
   FUNÇÃO: main
   Ponto de entrada do programa.
   Espera exatamente 1 argumento: o caminho do arquivo de template.
   ══════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    /* Verifica se o usuário passou o caminho do arquivo como argumento */
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.txt>\n", argv[0]); /* Exibe instrução de uso */
        return EXIT_FAILURE; /* Termina com código de erro (valor 1 no padrão POSIX)    */
    }

    SimuladorCompleto sim; /* Declara a struct que receberá todos os dados do arquivo */

    /* Tenta ler e interpretar o arquivo; encerra em caso de falha */
    if (ler_arquivo(argv[1], &sim) != 0)
        return EXIT_FAILURE; /* ler_arquivo já imprimiu a mensagem de erro adequada */

    imprimir_simulador(&sim); /* Exibe os dados lidos de forma organizada no terminal */

    return EXIT_SUCCESS; /* Termina com código 0 indicando execução bem-sucedida */
}
