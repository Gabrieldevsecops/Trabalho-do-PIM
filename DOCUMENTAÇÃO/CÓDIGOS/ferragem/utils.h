/*
 * utils.h
 *
 * Um arquivo .h (cabecalho) serve para "avisar" aos outros arquivos
 * quais funcoes existem. Quem escreve o codigo dessas funcoes e o utils.c.
 * Quem quiser usar, e so colocar #include "utils.h" la em cima.
 */

#ifndef UTILS_H
#define UTILS_H

void limpar_tela(void);                          /* apaga o que esta na tela      */
void pausar(void);                               /* espera o usuario dar ENTER    */
void ler_texto(char *destino, int tamanho);      /* le uma frase do teclado       */
int ler_inteiro(void);                           /* le um numero inteiro          */
double ler_decimal(void);                        /* le um numero com virgula/ponto */

#endif
