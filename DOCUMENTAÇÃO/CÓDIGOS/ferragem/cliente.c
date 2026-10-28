/*
 * cliente.c
 *
 * Tela do cliente. Ele nao faz login, so consulta.
 * Ve apenas: nome do produto, quantidade e preco, em ordem alfabetica.
 */

#include <stdio.h>
#include "banco.h"
#include "utils.h"
#include "cliente.h"


void tela_cliente(void) {
    sqlite3_stmt *consulta;

    limpar_tela();
    printf("==========================================\n");
    printf("         PRODUTOS DISPONIVEIS\n");
    printf("==========================================\n\n");

    /* ORDER BY nome = em ordem alfabetica
       (COLLATE NOCASE faz maiuscula e minuscula valerem igual) */
    sqlite3_prepare_v2(banco,
        "SELECT nome, quantidade, preco FROM produtos "
        "ORDER BY nome COLLATE NOCASE;",
        -1, &consulta, NULL);

    printf("%-28s %10s %12s\n", "PRODUTO", "QUANTIDADE", "PRECO (R$)");
    printf("--------------------------------------------------\n");

    /* Cada sqlite3_step pega uma linha da tabela. Quando acabam as
       linhas, ele para de devolver SQLITE_ROW e o while termina. */
    while (sqlite3_step(consulta) == SQLITE_ROW) {
        const char *nome = (const char *)sqlite3_column_text(consulta, 0);
        int quantidade   = sqlite3_column_int(consulta, 1);
        double preco     = sqlite3_column_double(consulta, 2);

        printf("%-28s %10d %12.2f\n", nome, quantidade, preco);
    }
    sqlite3_finalize(consulta);

    pausar();
}
