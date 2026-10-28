/*
 * login.c  -  TELA PRINCIPAL (e onde o programa comeca)
 *
 * Este e o arquivo que tem o main(). Ele:
 *   1. abre o banco e deixa tudo preparado (admin, tabelas)
 *   2. mostra o menu principal:
 *        1 - Login (funcionarios)
 *        2 - Ver tabela de produtos (cliente, sem login)
 *   3. depois do login, chama a tela certa de acordo com o perfil:
 *        admin              -> tela_admin()      (admin.c)
 *        gestor             -> tela_gestor()     (gestor.c)
 *        vendedor           -> tela_vendedor()   (vendedor.c)
 *        cliente            -> tela_cliente()    (cliente.c)
 *
 * Quando a outra tela termina, o programa volta para este menu.
 */

#include <stdio.h>
#include <string.h>
#include "banco.h"
#include "utils.h"
#include "cliente.h"
#include "vendedor.h"
#include "admin.h"
#include "gestor.h"


/* Procura no banco um usuario com esse login e essa senha.
   Se achar, copia o perfil dele (admin, gestor, vendedor) para "perfil"
   e devolve 1. Se nao achar, devolve 0. */
int verificar_login(const char *login, const char *senha, char *perfil) {
    sqlite3_stmt *consulta;
    int achou = 0;

    sqlite3_prepare_v2(banco,
        "SELECT perfil FROM usuarios WHERE login = ? AND senha = ?;",
        -1, &consulta, NULL);

    /* os "?" do comando acima sao trocados pelo login e pela senha */
    sqlite3_bind_text(consulta, 1, login, -1, SQLITE_STATIC);
    sqlite3_bind_text(consulta, 2, senha, -1, SQLITE_STATIC);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        snprintf(perfil, 20, "%s", (const char *)sqlite3_column_text(consulta, 0));
        achou = 1;
    }
    sqlite3_finalize(consulta);

    return achou;
}


/* Pede usuario e senha e, se estiverem certos, abre a tela do perfil */
void fazer_login(void) {
    char login[110];
    char senha[50];
    char perfil[20];

    printf("\nUsuario: ");
    ler_texto(login, sizeof(login));
    printf("Senha: ");
    ler_texto(senha, sizeof(senha));

    if (verificar_login(login, senha, perfil) == 0) {
        printf("\nUsuario ou senha incorretos.\n");
        pausar();
        return;
    }

     /* cada perfil abre a sua propria tela */
    if (strcmp(perfil, "vendedor") == 0) {
        tela_vendedor(login);
    } else if (strcmp(perfil, "gestor") == 0) {
        tela_gestor(login);
    } else if (strcmp(perfil, "admin") == 0) {
        tela_admin(login);
    }
}


int main(void) {
    int opcao;

    /* Preparacao do banco (codigo no banco.c) */
    abrir_banco();
    criar_tabelas();
    criar_admin_se_nao_existir();
    cadastrar_produtos_de_teste();

    /* Menu principal: repete ate o usuario escolher 0 */
    do {
        limpar_tela();
        printf("==========================================\n");
        printf("       CASA DE FERRAGENS - ESTOQUE\n");
        printf("==========================================\n");
        printf(" 1 - Fazer login \n");
        printf(" 2 - Ver tabela de produtos (clientes)\n");
        printf(" 0 - Sair\n");
        printf("------------------------------------------\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            fazer_login();
        } else if (opcao == 2) {
            tela_cliente();
        }
    } while (opcao != 0);

    fechar_banco();
    printf("\nSistema encerrado.\n");
    return 0;
}
