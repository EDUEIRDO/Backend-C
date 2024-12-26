# User CRUD Server

Este projeto implementa um servidor HTTP simples utilizando a biblioteca `libmicrohttpd` e um banco de dados SQLite para gerenciar usuários. O servidor oferece funcionalidades de criação de usuários (`POST /users`) e listagem de usuários (`GET /users`).

## Requisitos

Antes de rodar o projeto, você precisa garantir que as dependências necessárias estejam instaladas no seu sistema:

- **libmicrohttpd**: Biblioteca C para criar servidores HTTP.
- **SQLite3**: Banco de dados embutido.

### No Fedora (ou distribuições baseadas em Red Hat)

Para instalar as dependências, execute o seguinte comando:

```bash
sudo dnf install libmicrohttpd-devel sqlite-devel
```
## Como compilar

Após garantir que as dependências estão instaladas, compile o projeto utilizando o gcc com o seguinte comando:

```bash
gcc main.c database.c -o user-crud -lmicrohttpd -lsqlite3
```
