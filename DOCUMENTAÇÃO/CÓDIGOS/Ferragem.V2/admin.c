/*
Tela do administrador. Opcoes:
1 - Cadastrar usuario
2 - Usuarios         (mostra a lista de usuarios cadastrados)
3 - Editar usuarios  (muda nome, sobrenome, CPF, senha e nivel)
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "banco.h"
#include "utils.h"
#include "admin.h"


/* ================================================================
   QUANTOS DIGITOS DO CPF VIRAM A SENHA?
   Se quiser mudar (por exemplo para 5), troque so o numero abaixo.
   ================================================================ */
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


/* ================================================================
   AQUI A SENHA E CRIADA A PARTIR DO CPF.
   cpf aponta para o primeiro numero ("12345678909"). Somar um numero
   ao ponteiro "anda" essa quantidade de letras: cpf + 7 aponta para
   "8909". Entao "cpf + tamanho - 4" aponta para os 4 ultimos digitos.
   ================================================================ */
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

    printf("CPF (11 numeros, sem pontos nem tracos): ");
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


/* ================================================================
   OPCAO 2 - USUARIOS (lista)
   ================================================================ */

/* Copia o texto de uma coluna da consulta para "destino".
   Se a coluna estiver vazia (NULL no banco), copia um texto vazio. */
void copiar_coluna(sqlite3_stmt *consulta, int coluna, char *destino, int tamanho) {
    const char *texto = (const char *)sqlite3_column_text(consulta, coluna);

    if (texto == NULL) {
        texto = "";
    }
    snprintf(destino, tamanho, "%s", texto);
}


/* Escreve a lista de usuarios na tela (sem limpar a tela e sem pausar,
   assim da pra usar tanto em "Usuarios" quanto em "Editar usuarios").
   A senha nao aparece na lista de proposito. */
void mostrar_usuarios(void) {
    sqlite3_stmt *consulta;
    char login[110];
    char nome[50];
    char sobrenome[50];
    char cpf[12];
    char perfil[20];
    int id;
    int total = 0;

    sqlite3_prepare_v2(banco,
        "SELECT id, login, nome, sobrenome, cpf, perfil FROM usuarios ORDER BY id;",
        -1, &consulta, NULL);

    printf("%-4s %-20s %-13s %-14s %-12s %s\n",
           "ID", "LOGIN", "NOME", "SOBRENOME", "CPF", "NIVEL");
    printf("-------------------------------------------------------------------------\n");

    while (sqlite3_step(consulta) == SQLITE_ROW) {
        id = sqlite3_column_int(consulta, 0);
        copiar_coluna(consulta, 1, login,     sizeof(login));
        copiar_coluna(consulta, 2, nome,      sizeof(nome));
        copiar_coluna(consulta, 3, sobrenome, sizeof(sobrenome));
        copiar_coluna(consulta, 4, cpf,       sizeof(cpf));
        copiar_coluna(consulta, 5, perfil,    sizeof(perfil));

        /* o ".20" em %-20.20s corta o texto se ele for maior que 20 letras */
        printf("%-4d %-20.20s %-13.13s %-14.14s %-12s %s\n",
               id, login, nome,
               sobrenome[0] ? sobrenome : "-",
               cpf[0] ? cpf : "-",
               perfil);
        total++;
    }
    sqlite3_finalize(consulta);

    printf("\nTotal: %d usuario(s)\n", total);
}


/* Opcao 2 do menu: limpa a tela, mostra a lista e espera */
void ver_usuarios(void) {
    limpar_tela();
    printf("==========================================\n");
    printf("        USUARIOS CADASTRADOS\n");
    printf("==========================================\n\n");
    mostrar_usuarios();
    pausar();
}


/* ================================================================
   OPCAO 3 - EDITAR USUARIOS
   ================================================================ */

/* Procura um usuario pelo ID. Se achar, copia os dados dele para as
   variaveis recebidas e devolve 1. Se nao achar, devolve 0. */
int buscar_usuario(int id, char *login, char *nome, char *sobrenome,
                   char *cpf, char *perfil) {
    sqlite3_stmt *consulta;
    int achou = 0;

    sqlite3_prepare_v2(banco,
        "SELECT login, nome, sobrenome, cpf, perfil FROM usuarios WHERE id = ?;",
        -1, &consulta, NULL);
    sqlite3_bind_int(consulta, 1, id);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        copiar_coluna(consulta, 0, login,     110);
        copiar_coluna(consulta, 1, nome,      50);
        copiar_coluna(consulta, 2, sobrenome, 50);
        copiar_coluna(consulta, 3, cpf,       12);
        copiar_coluna(consulta, 4, perfil,    20);
        achou = 1;
    }
    sqlite3_finalize(consulta);

    return achou;
}


/* Igual a ja_existe, mas ignora o proprio usuario (o "id").
   Serve para o caso de o admin "trocar" um dado pelo mesmo valor que
   ja estava la: isso nao pode contar como duplicado. */
int ja_existe_outro(const char *coluna, const char *valor, int id) {
    sqlite3_stmt *consulta;
    char sql[120];
    int quantos = 0;

    snprintf(sql, sizeof(sql),
             "SELECT COUNT(*) FROM usuarios WHERE %s = ? AND id <> ?;", coluna);

    sqlite3_prepare_v2(banco, sql, -1, &consulta, NULL);
    sqlite3_bind_text(consulta, 1, valor, -1, SQLITE_STATIC);
    sqlite3_bind_int(consulta, 2, id);

    if (sqlite3_step(consulta) == SQLITE_ROW) {
        quantos = sqlite3_column_int(consulta, 0);
    }
    sqlite3_finalize(consulta);

    return quantos > 0;
}


/* Muda UMA coluna de texto de um usuario (nome, cpf, senha, perfil...).
   O nome da coluna vem sempre do nosso proprio codigo, nunca do teclado.
   Devolve 1 se deu certo, 0 se deu erro. */
int atualizar_usuario(int id, const char *coluna, const char *valor) {
    sqlite3_stmt *consulta;
    char sql[100];
    int deu_certo;

    snprintf(sql, sizeof(sql), "UPDATE usuarios SET %s = ? WHERE id = ?;", coluna);

    sqlite3_prepare_v2(banco, sql, -1, &consulta, NULL);
    sqlite3_bind_text(consulta, 1, valor, -1, SQLITE_STATIC);
    sqlite3_bind_int(consulta, 2, id);

    deu_certo = (sqlite3_step(consulta) == SQLITE_DONE);
    sqlite3_finalize(consulta);

    return deu_certo;
}


/* Mensagem de erro padrao quando o banco nao deixa gravar */
void avisar_erro_ao_gravar(void) {
    printf("\nErro ao gravar no banco.\n");
    printf("Se o DB Browser estiver aberto com mudancas sem salvar,\n");
    printf("clique em Escrever modificacoes ou feche ele e tente de novo.\n");
}


/* Grava o nome e o sobrenome novos e refaz o login a partir deles.
   Antes de gravar, confere se o login novo ja nao e de outra pessoa. */
void salvar_nome_e_login(int id, const char *nome, const char *sobrenome) {
    char login[110];

    montar_login(nome, sobrenome, login);

    if (ja_existe_outro("login", login, id)) {
        printf("\nJa existe outro usuario com o login %s.\n", login);
        printf("Nada foi alterado.\n");
        return;
    }

    if (atualizar_usuario(id, "nome", nome) &&
        atualizar_usuario(id, "sobrenome", sobrenome) &&
        atualizar_usuario(id, "login", login)) {
        printf("\nAtualizado!\n");
        printf("O novo login desse usuario e: %s\n", login);
    } else {
        avisar_erro_ao_gravar();
    }
}


void editar_nome(int id, const char *sobrenome_atual) {
    char novo[50];

    printf("Novo nome (sem acento, 0 para cancelar): ");
    ler_texto(novo, sizeof(novo));

    if (strcmp(novo, "0") == 0) {
        return;
    }
    if (novo[0] == '\0') {
        printf("\nO nome nao pode ficar vazio.\n");
        pausar();
        return;
    }

    salvar_nome_e_login(id, novo, sobrenome_atual);
    pausar();
}


void editar_sobrenome(int id, const char *nome_atual) {
    char novo[50];

    printf("Novo sobrenome (sem acento, 0 para cancelar): ");
    ler_texto(novo, sizeof(novo));

    if (strcmp(novo, "0") == 0) {
        return;
    }
    if (novo[0] == '\0') {
        printf("\nO sobrenome nao pode ficar vazio.\n");
        pausar();
        return;
    }

    salvar_nome_e_login(id, nome_atual, novo);
    pausar();
}


/* Troca o CPF. Depois pergunta se a senha deve voltar ao padrao
   (ultimos digitos do CPF novo), porque a senha antiga pode ter sido
   feita com o CPF antigo. */
void editar_cpf(int id) {
    char digitado[30];
    char cpf[12];
    char senha[12];
    int opcao;

    printf("Novo CPF (11 numeros, sem pontos nem tracos, 0 para cancelar): ");
    ler_texto(digitado, sizeof(digitado));

    if (strcmp(digitado, "0") == 0) {
        return;
    }
    if (limpar_cpf(digitado, cpf) == 0) {
        printf("\nCPF invalido. Precisa ter exatamente 11 numeros.\n");
        pausar();
        return;
    }
    if (ja_existe_outro("cpf", cpf, id)) {
        printf("\nJa existe outro usuario com esse CPF.\n");
        printf("Nada foi alterado.\n");
        pausar();
        return;
    }

    if (atualizar_usuario(id, "cpf", cpf) == 0) {
        avisar_erro_ao_gravar();
        pausar();
        return;
    }
    printf("\nCPF atualizado!\n");

    printf("\nQuer trocar a senha para os ultimos %d digitos do novo CPF?\n", DIGITOS_DA_SENHA);
    printf(" 1 - Sim\n");
    printf(" 0 - Nao, manter a senha atual\n");
    printf("Escolha: ");
    opcao = ler_inteiro();

    if (opcao == 1) {
        gerar_senha_do_cpf(cpf, senha);
        if (atualizar_usuario(id, "senha", senha)) {
            printf("\nSenha atualizada para: %s\n", senha);
        } else {
            avisar_erro_ao_gravar();
        }
    } else {
        printf("\nA senha continua a mesma.\n");
    }
    pausar();
}


/* Troca a senha por uma que o admin digitar (4 a 20 letras ou numeros,
   sem espacos). */
void editar_senha(int id) {
    char nova[50];
    int i;

    printf("Nova senha (4 a 20 caracteres, sem espacos, 0 para cancelar): ");
    ler_texto(nova, sizeof(nova));

    if (strcmp(nova, "0") == 0) {
        return;
    }
    if (strlen(nova) < 4 || strlen(nova) > 20) {
        printf("\nA senha precisa ter de 4 a 20 caracteres.\n");
        pausar();
        return;
    }
    for (i = 0; nova[i] != '\0'; i++) {
        if (nova[i] == ' ') {
            printf("\nA senha nao pode ter espacos.\n");
            pausar();
            return;
        }
    }

    if (atualizar_usuario(id, "senha", nova)) {
        printf("\nSenha atualizada!\n");
    } else {
        avisar_erro_ao_gravar();
    }
    pausar();
}


/* Troca o nivel entre vendedor e gestor */
void editar_nivel(int id) {
    int nivel;
    const char *perfil;

    printf("Novo nivel:\n");
    printf(" 1 - Vendedor\n");
    printf(" 2 - Gestor\n");
    printf(" 0 - Cancelar\n");
    printf("Escolha: ");
    nivel = ler_inteiro();

    if (nivel == 0) {
        return;
    }
    if (nivel == 1) {
        perfil = "vendedor";
    } else if (nivel == 2) {
        perfil = "gestor";
    } else {
        printf("\nNivel invalido. Nada foi alterado.\n");
        pausar();
        return;
    }

    if (atualizar_usuario(id, "perfil", perfil)) {
        printf("\nNivel atualizado para %s!\n", perfil);
    } else {
        avisar_erro_ao_gravar();
    }
    pausar();
}


/* Tela de um usuario so: mostra os dados dele e deixa escolher o que mudar.
   Os dados sao buscados de novo a cada volta, assim o que acabou de ser
   mudado ja aparece atualizado. */
void editar_um_usuario(int id) {
    char login[110];
    char nome[50];
    char sobrenome[50];
    char cpf[12];
    char perfil[20];
    int opcao;

    do {
        if (buscar_usuario(id, login, nome, sobrenome, cpf, perfil) == 0) {
            return;
        }

        limpar_tela();
        printf("==========================================\n");
        printf("            EDITAR USUARIO\n");
        printf("==========================================\n\n");
        printf("  Login:      %s\n", login);
        printf("  Nome:       %s %s\n", nome, sobrenome);
        printf("  CPF:        %s\n", cpf);
        printf("  Senha:      ****\n");
        printf("  Nivel:      %s\n", perfil);

        printf("\nO que voce quer mudar?\n");
        printf(" 1 - Nome\n");
        printf(" 2 - Sobrenome\n");
        printf(" 3 - CPF\n");
        printf(" 4 - Senha\n");
        printf(" 5 - Nivel (vendedor / gestor)\n");
        printf(" 0 - Voltar\n");
        printf("------------------------------------------\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao >= 1 && opcao <= 5) {
            printf("\n");
        }

        if (opcao == 1) {
            editar_nome(id, sobrenome);
        } else if (opcao == 2) {
            editar_sobrenome(id, nome);
        } else if (opcao == 3) {
            editar_cpf(id);
        } else if (opcao == 4) {
            editar_senha(id);
        } else if (opcao == 5) {
            editar_nivel(id);
        }
    } while (opcao != 0);
}


/* Opcao 3 do menu: mostra a lista e deixa escolher qual usuario editar */
void editar_usuarios(void) {
    char login[110];
    char nome[50];
    char sobrenome[50];
    char cpf[12];
    char perfil[20];
    int id;

    while (1) {
        limpar_tela();
        printf("==========================================\n");
        printf("            EDITAR USUARIOS\n");
        printf("==========================================\n\n");
        mostrar_usuarios();

        printf("\nDigite o ID do usuario que quer editar (0 para voltar): ");
        id = ler_inteiro();

        if (id == 0) {
            return;
        }

        if (id < 0 || buscar_usuario(id, login, nome, sobrenome, cpf, perfil) == 0) {
            printf("\nUsuario nao encontrado.\n");
            pausar();
            continue;
        }

        /* a conta admin nao se edita por aqui: se o nivel dela mudasse
           para vendedor ou gestor, ninguem mais conseguiria administrar */
        if (strcmp(perfil, "admin") == 0) {
            printf("\nA conta do administrador nao pode ser editada por aqui.\n");
            pausar();
            continue;
        }

        editar_um_usuario(id);
    }
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
        printf(" 2 - Usuarios\n");
        printf(" 3 - Editar usuarios\n");
        printf(" 0 - Sair\n");
        printf("------------------------------------------\n");
        printf("Escolha: ");
        opcao = ler_inteiro();

        if (opcao == 1) {
            cadastrar_usuario();
        } else if (opcao == 2) {
            ver_usuarios();
        } else if (opcao == 3) {
            editar_usuarios();
        }
    } while (opcao != 0);
}
