/*
 * vendedor.c
 *
 * Tela do vendedor (o gestor usa a mesma, pois tem os mesmos acessos).
 *
 * Menu:
 *   1 - Ver tabela de itens   (alem do que o cliente ve, mostra o ID,
 *                              o estoque minimo e um aviso de situacao)
 *   2 - Editar tabela         (mudar preco ou quantidade de um produto)
 */

#include <stdio.h>
#include "banco.h"
#include "utils.h"
#include "vendedor.h"


/* Conta quantos produtos estao abaixo do minimo (ou zerados) */
int contar_produtos_em_aviso(void) {
    sqlite3_stmt *consulta;
    int total = 0;

    sqlite3_prepare_v2(banco,
        "SELECT COUNT(*) FROM produtos WHERE quantidade < estoque_minimo;",
        -1, &consulta, NULL);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        total = sqlite3_column_int(consulta, 0);
    }
    sqlite3_finalize(consulta);

    return total;
}


/* Escreve a tabela na tela (sem limpar a tela e sem pausar,
   assim da pra usar tanto em "ver" quanto em "editar") */
void mostrar_tabela(void) {
    sqlite3_stmt *consulta;

    sqlite3_prepare_v2(banco,
        "SELECT id, nome, quantidade, estoque_minimo, preco FROM produtos "
        "ORDER BY nome COLLATE NOCASE;",
        -1, &consulta, NULL);

    printf("%-4s %-26s %6s %6s %10s   %s\n",
           "ID", "PRODUTO", "QTD", "MIN", "PRECO (R$)", "SITUACAO");
    printf("---------------------------------------------------------------------\n");

    while (sqlite3_step(consulta) == SQLITE_ROW) {
        int id             = sqlite3_column_int(consulta, 0);
        const char *nome   = (const char *)sqlite3_column_text(consulta, 1);
        int quantidade     = sqlite3_column_int(consulta, 2);
        int minimo         = sqlite3_column_int(consulta, 3);
        double preco       = sqlite3_column_double(consulta, 4);

        printf("%-4d %-26s %6d %6d %10.2f   ", id, nome, quantidade, minimo, preco);

        /* O aviso que so o vendedor ve */
        if (quantidade == 0) {
            printf("SEM ESTOQUE");
        } else if (quantidade < minimo) {
            printf("ABAIXO DO MINIMO");
        } else {
            printf("ok");
        }
        printf("\n");
    }
    sqlite3_finalize(consulta);
}


/* Opcao 1 do menu: limpa a tela, mostra a tabela e espera */
void ver_tabela(void) {
    limpar_tela();
    printf("==========================================\n");
    printf("            TABELA DE ITENS\n");
    printf("==========================================\n\n");
    mostrar_tabela();
    pausar();
}


/* Procura um produto pelo ID. Se achar, copia o nome para "nome" e
   devolve 1. Se nao achar, devolve 0. */
int buscar_nome_do_produto(int id, char *nome) {
    sqlite3_stmt *consulta;
    int achou = 0;

    sqlite3_prepare_v2(banco,
        "SELECT nome FROM produtos WHERE id = ?;",
        -1, &consulta, NULL);
    sqlite3_bind_int(consulta, 1, id);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        snprintf(nome, 100, "%s", (const char *)sqlite3_column_text(consulta, 0));
        achou = 1;
    }
    sqlite3_finalize(consulta);

    return achou;
}


/* Muda o nome de um produto */
void mudar_nome(int id) {
    sqlite3_stmt *consulta;
    char novo_nome[100];

    printf("Novo nome: ");
    ler_texto(novo_nome, sizeof(novo_nome));

    if (novo_nome[0] == '\0') {
        printf("\nO nome nao pode ficar vazio. Nada foi alterado.\n");
        return;
    }

    sqlite3_prepare_v2(banco,
        "UPDATE produtos SET nome = ? WHERE id = ?;",
        -1, &consulta, NULL);

    sqlite3_bind_text(consulta, 1, novo_nome, -1, SQLITE_STATIC);
    sqlite3_bind_int(consulta, 2, id);

    if (sqlite3_step(consulta) == SQLITE_DONE) {
        printf("\nNome atualizado com sucesso!\n");
    } else {
        printf("\nErro ao atualizar o nome no banco.\n");
    }
    sqlite3_finalize(consulta);
}


/* Muda o preco de um produto */
void mudar_preco(int id) {
    char sql[200];
    double novo_preco;

    printf("Novo preco: ");
    novo_preco = ler_decimal();

    if (novo_preco <= 0) {
        printf("\nPreco invalido. Nada foi alterado.\n");
        return;
    }

    /* monta o comando SQL colocando o preco e o id dentro do texto */
    snprintf(sql, sizeof(sql),
             "UPDATE produtos SET preco = %.2f WHERE id = %d;", novo_preco, id);
    sqlite3_exec(banco, sql, NULL, NULL, NULL);

    printf("\nPreco atualizado!\n");
}


/* Muda a quantidade em estoque de um produto */
void mudar_quantidade(int id) {
    char sql[200];
    int nova_quantidade;

    printf("Nova quantidade em estoque: ");
    nova_quantidade = ler_inteiro();

    if (nova_quantidade < 0) {
        printf("\nQuantidade invalida. Nada foi alterado.\n");
        return;
    }

    snprintf(sql, sizeof(sql),
             "UPDATE produtos SET quantidade = %d WHERE id = %d;", nova_quantidade, id);
    sqlite3_exec(banco, sql, NULL, NULL, NULL);

    printf("\nQuantidade atualizada!\n");
}


/* Opcao 2 do menu: escolher um produto e mudar preco ou quantidade */
/* Opcao 2 do menu: escolher um produto e mudar nome, preco ou quantidade */
void editar_tabela(void) {
    int id;
    int opcao;
    char nome[100];

    while (1) {
        limpar_tela();
        printf("==========================================\n");
        printf("            EDITAR TABELA\n");
        printf("==========================================\n\n");
        mostrar_tabela();

        printf("\nDigite o ID do produto que quer editar (0 para voltar): ");
        id = ler_inteiro();

        if (id == 0) {
            return;
        }

        if (id < 0 || buscar_nome_do_produto(id, nome) == 0) {
            printf("\nProduto nao encontrado.\n");
            pausar();
            continue;   /* volta pro comeco do while */
        }

        printf("\nProduto escolhido: %s\n", nome);
        printf(" 1 - Mudar nome\n");
        printf(" 2 - Mudar preco\n");
        printf(" 3 - Mudar quantidade em estoque\n");
        printf(" 0 - Voltar\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            mudar_nome(id);
            pausar();
        } else if (opcao == 2) {
            mudar_preco(id);
            pausar();
        } else if (opcao == 3) {
            mudar_quantidade(id);
            pausar();
        }
    }
}


/* A tela do vendedor: mostra o menu e chama a funcao da opcao escolhida.
   Quando o vendedor escolhe 0, a funcao termina e o programa volta
   para quem chamou (a tela de login). */
void tela_vendedor(const char *nome_usuario) {
    int opcao;
    int em_aviso;

    do {
        limpar_tela();
        printf("==========================================\n");
        printf("            TELA DO VENDEDOR\n");
        printf("==========================================\n");
        printf("Usuario: %s\n", nome_usuario);

        em_aviso = contar_produtos_em_aviso();
        if (em_aviso > 0) {
            printf("\nATENCAO: %d produto(s) abaixo do minimo ou sem estoque!\n", em_aviso);
        }

        printf("\n 1 - Ver tabela de itens\n");
        printf(" 2 - Editar tabela\n");
        printf(" 0 - Sair\n");
        printf("------------------------------------------\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            ver_tabela();
        } else if (opcao == 2) {
            editar_tabela();
        }
    } while (opcao != 0);
}
