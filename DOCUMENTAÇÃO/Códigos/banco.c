/*
 * banco.c
 *
 * Tudo que prepara o banco de dados (arquivo estoque.db):
 *   - abre o banco (se o arquivo nao existir, o SQLite cria sozinho)
 *   - cria as tabelas de usuarios e de produtos
 *   - confere se o admin existe e cria se precisar
 *   - coloca alguns produtos de teste
 *
 * O estoque.db fica na mesma pasta do projeto.
 */

#include <stdio.h>
#include <stdlib.h>
#include "banco.h"

/* A variavel que representa o banco aberto. Todos os arquivos usam ela. */
sqlite3 *banco;


void abrir_banco(void) {
    int resultado = sqlite3_open("estoque.db", &banco);

    if (resultado != SQLITE_OK) {
        printf("Nao foi possivel abrir o banco de dados.\n");
        exit(1);
    }
}


void fechar_banco(void) {
    sqlite3_close(banco);
}


/* Cria as duas tabelas. O "IF NOT EXISTS" evita erro se elas ja existirem. */
void criar_tabelas(void) {
    /* Cada usuario e uma LINHA desta tabela. As COLUNAS sao os campos. */
    const char *sql_usuarios =
        "CREATE TABLE IF NOT EXISTS usuarios ("
        "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  login     TEXT NOT NULL UNIQUE,"    /* o que a pessoa digita pra entrar */
        "  nome      TEXT NOT NULL,"
        "  sobrenome TEXT,"
        "  cpf       TEXT UNIQUE,"             /* so os 11 numeros, sem ponto */
        "  senha     TEXT NOT NULL,"
        "  perfil    TEXT NOT NULL"            /* admin, gestor ou vendedor */
        ");";

    const char *sql_produtos =
        "CREATE TABLE IF NOT EXISTS produtos ("
        "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  nome           TEXT NOT NULL,"
        "  preco          REAL NOT NULL,"
        "  quantidade     INTEGER NOT NULL,"
        "  estoque_minimo INTEGER NOT NULL"  /* abaixo disso vira aviso */
        ");";

    if (sqlite3_exec(banco, sql_usuarios, NULL, NULL, NULL) != SQLITE_OK) {
        printf("Erro ao criar a tabela de usuarios.\n");
        exit(1);
    }
    if (sqlite3_exec(banco, sql_produtos, NULL, NULL, NULL) != SQLITE_OK) {
        printf("Erro ao criar a tabela de produtos.\n");
        exit(1);
    }

    /* Se o estoque.db foi criado por uma versao antiga do programa, a tabela
       usuarios nao tem as colunas novas (login, sobrenome, cpf).
       O IF NOT EXISTS nao atualiza tabela velha, entao avisamos aqui. */
    if (sqlite3_exec(banco, "SELECT login, sobrenome, cpf FROM usuarios LIMIT 1;",
                     NULL, NULL, NULL) != SQLITE_OK) {
        printf("O estoque.db e de uma versao antiga do programa.\n");
        printf("Feche o DB Browser, apague o arquivo estoque.db e rode de novo.\n");
        exit(1);
    }
}


/* Procura o usuario admin no banco. Se nao achar, cria com senha admin123. */
void criar_admin_se_nao_existir(void) {
    sqlite3_stmt *consulta;
    int quantos = 0;

    sqlite3_prepare_v2(banco,
        "SELECT COUNT(*) FROM usuarios WHERE login = 'admin';",
        -1, &consulta, NULL);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        quantos = sqlite3_column_int(consulta, 0);
    }
    sqlite3_finalize(consulta);

    if (quantos == 0) {
        sqlite3_exec(banco,
            "INSERT INTO usuarios (login, nome, senha, perfil) "
            "VALUES ('admin', 'Administrador', 'admin123', 'admin');",
            NULL, NULL, NULL);
    }
}


/* So para testar: se nao tiver nenhum produto, coloca alguns.
   Quando a tela do admin estiver pronta (cadastro de produtos),
   essa funcao pode ser apagada. */
void cadastrar_produtos_de_teste(void) {
    sqlite3_stmt *consulta;
    int quantos = 0;

    sqlite3_prepare_v2(banco, "SELECT COUNT(*) FROM produtos;", -1, &consulta, NULL);
    if (sqlite3_step(consulta) == SQLITE_ROW) {
        quantos = sqlite3_column_int(consulta, 0);
    }
    sqlite3_finalize(consulta);

    if (quantos > 0) {
        return;   /* ja tem produtos, nao faz nada */
    }

    sqlite3_exec(banco,
        "INSERT INTO produtos (nome, preco, quantidade, estoque_minimo) VALUES"
        " ('Martelo 25mm',       32.90, 18,   5),"
        " ('Chave de fenda',     12.50,  3,   8),"
        " ('Prego 18x30 (kg)',   14.00, 40,  10),"
        " ('Parafuso sextavado',  0.80,  0, 100),"
        " ('Tinta branca 18L',  289.00,  6,   4),"
        " ('Rolo de pintura',    15.90,  4,   6),"
        " ('Cadeado 35mm',       22.00, 11,   5),"
        " ('Fita isolante',       6.50,  2,  10);",
        NULL, NULL, NULL);
}
