/*
 * gestor.c
 *
 * Tela do gestor. Ele tem tudo que o vendedor tem e mais duas coisas:
 *   1 - Ver tabela de itens      (funcao que ja existe no vendedor.c)
 *   2 - Editar tabela            (funcao que ja existe no vendedor.c)
 *   3 - Cadastrar produto novo
 *   4 - Remover produto
 *
 * Repare que as opcoes 1 e 2 NAO foram copiadas para ca: este arquivo
 * chama as funcoes do vendedor.c (por isso o #include "vendedor.h").
 * Um codigo chamando o outro.
 */

#include <stdio.h>
#include <string.h>
#include "banco.h"
#include "utils.h"
#include "vendedor.h"
#include "gestor.h"


/* Confere se ja existe um produto com esse nome
   (maiuscula e minuscula valem igual). Devolve 1 se existe, 0 se nao. */
int produto_ja_existe(const char *nome) {
    sqlite3_stmt *consulta;
    int quantos = 0;

    sqlite3_prepare_v2(banco,
        "SELECT COUNT(*) FROM produtos WHERE nome = ? COLLATE NOCASE;",
        -1, &consulta, NULL);
    sqlite3_bind_text(consulta, 1, nome, -1, SQLITE_STATIC);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        quantos = sqlite3_column_int(consulta, 0);
    }
    sqlite3_finalize(consulta);

    return quantos > 0;
}


/* Grava o produto novo no banco. Devolve 1 se deu certo, 0 se deu erro. */
int gravar_produto(const char *nome, double preco, int quantidade, int minimo) {
    sqlite3_stmt *consulta;
    int deu_certo;

    sqlite3_prepare_v2(banco,
        "INSERT INTO produtos (nome, preco, quantidade, estoque_minimo) "
        "VALUES (?, ?, ?, ?);",
        -1, &consulta, NULL);

    /* cada "?" acima e trocado por um valor, na ordem (1, 2, 3, 4) */
    sqlite3_bind_text(consulta, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_double(consulta, 2, preco);
    sqlite3_bind_int(consulta, 3, quantidade);
    sqlite3_bind_int(consulta, 4, minimo);

    deu_certo = (sqlite3_step(consulta) == SQLITE_DONE);
    sqlite3_finalize(consulta);

    return deu_certo;
}


/* Opcao 3 do menu: pede os dados do produto e grava no banco */
void cadastrar_produto(void) {
    char nome[50];
    double preco;
    int quantidade;
    int minimo;

    limpar_tela();
    printf("==========================================\n");
    printf("         CADASTRAR PRODUTO NOVO\n");
    printf("==========================================\n\n");

    printf("Nome do produto (0 para cancelar): ");
    ler_texto(nome, sizeof(nome));
    if (strcmp(nome, "0") == 0) {
        return;
    }
    if (nome[0] == '\0') {
        printf("\nO nome nao pode ficar vazio.\n");
        pausar();
        return;
    }
    if (produto_ja_existe(nome)) {
        printf("\nJa existe um produto com esse nome.\n");
        printf("Para mudar preco ou quantidade, use a opcao Editar tabela.\n");
        pausar();
        return;
    }

    printf("Preco (ex: 12,50): ");
    preco = ler_decimal();
    if (preco <= 0) {
        printf("\nPreco invalido. Produto nao cadastrado.\n");
        pausar();
        return;
    }

    printf("Quantidade em estoque: ");
    quantidade = ler_inteiro();
    if (quantidade < 0) {
        printf("\nQuantidade invalida. Produto nao cadastrado.\n");
        pausar();
        return;
    }

    printf("Estoque minimo (abaixo disso aparece o aviso): ");
    minimo = ler_inteiro();
    if (minimo < 0) {
        printf("\nEstoque minimo invalido. Produto nao cadastrado.\n");
        pausar();
        return;
    }

    if (gravar_produto(nome, preco, quantidade, minimo)) {
        printf("\nProduto cadastrado com sucesso!\n");
    } else {
        printf("\nErro ao gravar o produto no banco.\n");
        printf("Se o DB Browser estiver aberto com mudancas sem salvar,\n");
        printf("clique em Escrever modificacoes ou feche ele e tente de novo.\n");
    }
    pausar();
}


/* Apaga o produto do banco. Devolve 1 se deu certo, 0 se deu erro. */
int apagar_produto(int id) {
    sqlite3_stmt *consulta;
    int deu_certo;

    sqlite3_prepare_v2(banco, "DELETE FROM produtos WHERE id = ?;", -1, &consulta, NULL);
    sqlite3_bind_int(consulta, 1, id);

    deu_certo = (sqlite3_step(consulta) == SQLITE_DONE);
    sqlite3_finalize(consulta);

    return deu_certo;
}


/* Opcao 4 do menu: escolher um produto e remover (com confirmacao,
   porque depois de apagado nao tem como voltar) */
void remover_produto(void) {
    int id;
    int opcao;
    char nome[100];

    while (1) {
        limpar_tela();
        printf("==========================================\n");
        printf("            REMOVER PRODUTO\n");
        printf("==========================================\n\n");
        mostrar_tabela();     /* funcao do vendedor.c */

        printf("\nDigite o ID do produto que quer remover (0 para voltar): ");
        id = ler_inteiro();

        if (id == 0) {
            return;
        }

        if (id < 0 || buscar_nome_do_produto(id, nome) == 0) {
            printf("\nProduto nao encontrado.\n");
            pausar();
            continue;
        }

        printf("\nVoce vai remover: %s\n", nome);
        printf("Isso nao tem volta. Tem certeza?\n");
        printf(" 1 - Sim, remover\n");
        printf(" 0 - Nao, cancelar\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            if (apagar_produto(id)) {
                printf("\nProduto removido!\n");
            } else {
                printf("\nErro ao remover o produto.\n");
            }
        } else {
            printf("\nNada foi removido.\n");
        }
        pausar();
    }
}


/* A tela do gestor: mostra o menu e chama a funcao da opcao escolhida.
   Quando o gestor escolhe 0, volta para a tela de login. */
void tela_gestor(const char *nome_usuario) {
    int opcao;
    int em_aviso;

    do {
        limpar_tela();
        printf("==========================================\n");
        printf("             TELA DO GESTOR\n");
        printf("==========================================\n");
        printf("Usuario: %s\n", nome_usuario);

        em_aviso = contar_produtos_em_aviso();    /* funcao do vendedor.c */
        if (em_aviso > 0) {
            printf("\nATENCAO: %d produto(s) abaixo do minimo ou sem estoque!\n", em_aviso);
        }

        printf("\n 1 - Ver tabela de itens\n");
        printf(" 2 - Editar tabela\n");
        printf(" 3 - Cadastrar produto novo\n");
        printf(" 4 - Remover produto\n");
        printf(" 0 - Sair\n");
        printf("------------------------------------------\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            limpar_tela();
            printf("==========================================\n");
            printf("            TABELA DE ITENS\n");
            printf("==========================================\n\n");
            mostrar_tabela();
            pausar();
        } else if (opcao == 2) {
            editar_tabela();         /* funcao do vendedor.c */
        } else if (opcao == 3) {
            cadastrar_produto();
        } else if (opcao == 4) {
            remover_produto();
        }
    } while (opcao != 0);
}
