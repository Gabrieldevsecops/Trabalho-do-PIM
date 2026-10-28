/*
 * admin.c
 *
 * Tela do administrador. Por enquanto tem so uma opcao:
 *   1 - Cadastrar usuario
 *
 * Ao cadastrar, o admin informa nome, sobrenome, CPF e o nivel
 * (vendedor ou gestor). O programa cria sozinho:
 *   - o LOGIN:  nome.sobrenome em letras minusculas (ex: joao.silva)
 *   - a SENHA:  os ultimos digitos do CPF (ex: 8909)
 * Assim ninguem precisa escolher nem esquecer senha.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "banco.h"
#include "utils.h"
#include "admin.h"


/* Quantos digitos vao ser senha,s e quiser mudar (por exemplo para 5), troque so o numero abaixo.*/
#define DIGITOS_DA_SENHA 4


/* Confere se o CPF digitado tem exatamente 11 numeros (sem ponto e sem traco).
   Se estiver certo, copia para "limpo" e devolve 1. Senao, devolve 0.
   "limpo" precisa ter espaco para 12 letras. */
int limpar_cpf(const char *digitado, char *limpo) {
    int i;

    for (i = 0; digitado[i] != '\0'; i++) {
        if (!isdigit((unsigned char)digitado[i])) {
            return 0;                /* tem letra, ponto, traco ou espaco */
        }
    }

    if (i != 11) {
        return 0;                    /* nao tem 11 numeros */
    }

    strcpy(limpo, digitado);
    return 1;
}


/* Senha criada usando o cpf, cpf aponta para o primeiro numero ("12345678909"). Somar um numero
   ao ponteiro "anda" essa quantidade de letras: cpf + 7 aponta para
   "8909". Entao "cpf + tamanho - 4" aponta para os 4 ultimos digitos. */

void gerar_senha_do_cpf(const char *cpf, char *senha) {
    int tamanho = strlen(cpf);

    snprintf(senha, 12, "%s", cpf + tamanho - DIGITOS_DA_SENHA);
}


/* Monta o login: nome + ponto + sobrenome, tudo minusculo e sem espacos.
   Exemplo: "Joao" e "Silva" viram "joao.silva".
   "login" precisa ter espaco para 100 letras ou mais. */

void montar_login(const char *nome, const char *sobrenome, char *login) {
    int i;
    int j = 0;

    for (i = 0; nome[i] != '\0'; i++) {
        if (nome[i] != ' ') {
            login[j] = tolower((unsigned char)nome[i]);
            j++;
        }
    }

    login[j] = '.';
    j++;

    for (i = 0; sobrenome[i] != '\0'; i++) {
        if (sobrenome[i] != ' ') {
            login[j] = tolower((unsigned char)sobrenome[i]);
            j++;
        }
    }

    login[j] = '\0';
}


/* Confere se ja existe alguem com esse valor na coluna (login ou cpf).
   Devolve 1 se existe e 0 se nao existe. */

int ja_existe(const char *coluna, const char *valor) {
    sqlite3_stmt *consulta;
    char sql[100];
    int quantos = 0;

    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM usuarios WHERE %s = ?;", coluna);

    sqlite3_prepare_v2(banco, sql, -1, &consulta, NULL);
    sqlite3_bind_text(consulta, 1, valor, -1, SQLITE_STATIC);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        quantos = sqlite3_column_int(consulta, 0);
    }
    sqlite3_finalize(consulta);

    return quantos > 0;
}


/* Grava o usuario novo no banco. Devolve 1 se deu certo, 0 se deu erro. */

int gravar_usuario(const char *login, const char *nome, const char *sobrenome,
                   const char *cpf, const char *senha, const char *perfil) {
    sqlite3_stmt *consulta;
    int deu_certo;

    sqlite3_prepare_v2(banco,
        "INSERT INTO usuarios (login, nome, sobrenome, cpf, senha, perfil) "
        "VALUES (?, ?, ?, ?, ?, ?);",
        -1, &consulta, NULL);

    /* cada "?" acima e trocado por um valor, na ordem (1, 2, 3...) */
    sqlite3_bind_text(consulta, 1, login,     -1, SQLITE_STATIC);
    sqlite3_bind_text(consulta, 2, nome,      -1, SQLITE_STATIC);
    sqlite3_bind_text(consulta, 3, sobrenome, -1, SQLITE_STATIC);
    sqlite3_bind_text(consulta, 4, cpf,       -1, SQLITE_STATIC);
    sqlite3_bind_text(consulta, 5, senha,     -1, SQLITE_STATIC);
    sqlite3_bind_text(consulta, 6, perfil,    -1, SQLITE_STATIC);

    deu_certo = (sqlite3_step(consulta) == SQLITE_DONE);
    sqlite3_finalize(consulta);

    return deu_certo;
}


/* Opcao 1 do menu: pede os dados, cria login e senha e grava no banco */

void cadastrar_usuario(void) {
    char nome[50];
    char sobrenome[50];
    char cpf_digitado[30];
    char cpf[12];
    char login[110];
    char senha[12];
    const char *perfil;
    int nivel;

    limpar_tela();
    printf("==========================================\n");
    printf("           CADASTRAR USUARIO\n");
    printf("==========================================\n\n");

    printf("Nome (sem acento, 0 para cancelar): ");
    ler_texto(nome, sizeof(nome));
    if (strcmp(nome, "0") == 0) {
        return;
    }
    if (nome[0] == '\0') {
        printf("\nO nome nao pode ficar vazio.\n");
        pausar();
        return;
    }

    printf("Sobrenome (sem acento): ");
    ler_texto(sobrenome, sizeof(sobrenome));
    if (sobrenome[0] == '\0') {
        printf("\nO sobrenome nao pode ficar vazio.\n");
        pausar();
        return;
    }

    printf("CPF (11 numeros, sem pontos e traços): ");
    ler_texto(cpf_digitado, sizeof(cpf_digitado));
    if (limpar_cpf(cpf_digitado, cpf) == 0) {
        printf("\nCPF invalido. Precisa ter exatamente 11 numeros.\n");
        pausar();
        return;
    }

    printf("\nNivel do usuario:\n");
    printf(" 1 - Vendedor\n");
    printf(" 2 - Gestor\n");
    printf("Escolha: ");
    nivel = ler_inteiro();

    if (nivel == 1) {
        perfil = "vendedor";
    } else if (nivel == 2) {
        perfil = "gestor";
    } else {
        printf("\nNivel invalido. Usuario nao cadastrado.\n");
        pausar();
        return;
    }

    /* a senha e o login nascem automaticamente */
    gerar_senha_do_cpf(cpf, senha);
    montar_login(nome, sobrenome, login);

    if (ja_existe("login", login)) {
        printf("\nJa existe um usuario com o login %s.\n", login);
        printf("Nada foi cadastrado.\n");
        pausar();
        return;
    }
    if (ja_existe("cpf", cpf)) {
        printf("\nJa existe um usuario com esse CPF.\n");
        printf("Nada foi cadastrado.\n");
        pausar();
        return;
    }

    if (gravar_usuario(login, nome, sobrenome, cpf, senha, perfil)) {
        printf("\nUsuario cadastrado com sucesso!\n\n");
        printf("  Nome:   %s %s\n", nome, sobrenome);
        printf("  Nivel:  %s\n", perfil);
        printf("  Login:  %s\n", login);
        printf("  Senha:  %s (ultimos %d digitos do CPF)\n", senha, DIGITOS_DA_SENHA);
    } else {
        printf("\nErro ao gravar o usuario no banco.\n");
        printf("Se o DB Browser estiver aberto com mudancas sem salvar,\n");
        printf("clique em Write Changes ou feche ele e tente de novo.\n");
    }
    pausar();
}


/* A tela do admin: mostra o menu e chama a funcao da opcao escolhida.
   Quando o admin escolhe 0, volta para a tela de login. */

void tela_admin(const char *nome_usuario) {
    int opcao;

    do {
        limpar_tela();
        printf("==========================================\n");
        printf("         TELA DO ADMINISTRADOR\n");
        printf("==========================================\n");
        printf("Usuario: %s\n\n", nome_usuario);
        printf(" 1 - Cadastrar usuario\n");
        printf(" 0 - Sair\n");
        printf("------------------------------------------\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            cadastrar_usuario();
        }
    } while (opcao != 0);
}
