/*
 * utils.c
 *
 * Funcoes pequenas que todas as telas usam (limpar tela, ler do teclado...).
 * Ficam aqui para nao repetir o mesmo codigo em cada arquivo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"


/* Apaga a tela. No Windows o comando e "cls", no Linux/Mac e "clear". */
void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


/* Le uma frase do teclado e tira o ENTER que vem no final.
   Se o teclado "acabar" (fgets devolve NULL), o programa fecha. */
void ler_texto(char *destino, int tamanho) {
    if (fgets(destino, tamanho, stdin) == NULL) {
        exit(0);
    }
    destino[strcspn(destino, "\n")] = '\0';
}


/* Mostra uma mensagem e espera o usuario apertar ENTER */
void pausar(void) {
    char lixo[10];
    printf("\nAperte ENTER para continuar...");
    ler_texto(lixo, sizeof(lixo));
}


/* Le um numero inteiro (0, 1, 25...).
   Se o usuario digitar qualquer outra coisa, devolve -1. */
int ler_inteiro(void) {
    char texto[30];
    int i;

    ler_texto(texto, sizeof(texto));

    if (texto[0] == '\0') {
        return -1;
    }
    for (i = 0; texto[i] != '\0'; i++) {
        if (!isdigit((unsigned char)texto[i])) {
            return -1;
        }
    }
    return atoi(texto);
}


/* Le um numero com casas decimais (12.50 ou 12,50).
   Se o usuario digitar qualquer outra coisa, devolve -1. */
double ler_decimal(void) {
    char texto[30];
    int i;
    int pontos = 0;

    ler_texto(texto, sizeof(texto));

    if (texto[0] == '\0') {
        return -1;
    }
    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] == ',') {
            texto[i] = '.';          /* troca virgula por ponto */
        }
        if (texto[i] == '.') {
            pontos++;
        } else if (!isdigit((unsigned char)texto[i])) {
            return -1;
        }
    }
    if (pontos > 1) {
        return -1;
    }
    return atof(texto);
}
