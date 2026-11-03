# Trabalho-do-PIM
Estamos utilizando esse repositório para documentar todo o processo do nosso trabalho da faculdade de análise e desenvolvimento de sistemas 
# Sistema de Controle de Estoque - ConstruMais

Projeto desenvolvido em linguagem C integrado ao banco de dados SQLite para gerenciamento de estoque e controle de acesso por perfis de usuário.

---

## 👥 Perfis de Acesso

- **Administrador:** Cadastro, listagem e edição de dados e permissões dos usuários do sistema.
- **Gestor:** Visualização detalhada do estoque, alertas de itens abaixo do mínimo, edição de valores/quantidades, cadastro e exclusão de produtos.
- **Vendedor:** Consulta à tabela completa com alertas de reposição e atualização de preços e quantidades.
- **Cliente:** Acesso público (sem necessidade de login) para consulta de produtos disponíveis e preços.

---

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C
- **Banco de Dados:** SQLite3 (Amalgamation)
- **Compilador sugerido:** GCC / MinGW

---

## 🚀 Como Compilar e Rodar

Abra o terminal na pasta do projeto e execute:

```bash
# Compilar todos os arquivos juntos com a biblioteca do SQLite
gcc -o estoque login.c admin.c banco.c cliente.c gestor.c vendedor.c utils.c sqlite3.c

# Executar o programa
./estoque
<img width="1570" height="816" alt="WhatsApp Image 2026-11-02 at 22 51 49" src="https://github.com/user-attachments/assets/b4cd30eb-3b07-44e6-a805-b3f5023b46bc" />
