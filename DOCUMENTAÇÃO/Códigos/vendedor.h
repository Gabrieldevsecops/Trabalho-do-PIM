/*
 * vendedor.h
 * Avisa quais funcoes do vendedor.c podem ser usadas por outros arquivos.
 */

#ifndef VENDEDOR_H
#define VENDEDOR_H

void tela_vendedor(const char *nome_usuario);
void mostrar_tabela(void);
void editar_tabela(void);
int buscar_nome_do_produto(int id, char *nome);
int contar_produtos_em_aviso(void);

#endif
