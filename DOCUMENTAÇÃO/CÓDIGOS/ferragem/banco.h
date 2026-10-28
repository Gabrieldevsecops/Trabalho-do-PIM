/*
 * banco.h
 *
 * Avisa quais funcoes do banco de dados existem (codigo no banco.c).
 *
 * "extern sqlite3 *banco" quer dizer: a variavel banco foi criada no
 * banco.c, mas todos os arquivos que incluirem este .h podem usar ela.
 */

#ifndef BANCO_H
#define BANCO_H

#include "sqlite3.h"

extern sqlite3 *banco;

void abrir_banco(void);
void fechar_banco(void);
void criar_tabelas(void);
void criar_admin_se_nao_existir(void);
void cadastrar_produtos_de_teste(void);

#endif
