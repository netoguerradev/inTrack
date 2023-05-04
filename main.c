#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int callback(void *, int, char **, char **);
int authenticateManager(int rc, sqlite3 *db, char *err_msg);

struct Manager{
    int id;
    char name[200];
    char password[200];
};

struct Manager *array[50];
int contManager = 0;

int main(void) {
    int autenticacao, tipo_de_usuario;
    sqlite3 *db;
    char *err_msg = 0;
    
    int rc = sqlite3_open("test.db", &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    
    char *sql = "CREATE TABLE IF NOT EXISTS managers(id INTEGER PRIMARY KEY, name TEXT, password TEXT);"
                "CREATE TABLE IF NOT EXISTS preceptors(id INTEGER PRIMARY KEY, name TEXT, password TEXT);"
                "CREATE TABLE IF NOT EXISTS residents(id INTEGER PRIMARY KEY, name TEXT, password TEXT);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        return 1;
    } 

    printf("Faca seu login!\nSe você possui uma conta, digite 0, senão 1 para criar uma nova. \nValor: ");
    scanf("%i", &autenticacao);

    int status = 0;

    if (autenticacao == 0) {
        printf("\nSe você e da gestão aperte: 1, Preceptor: 2 e Residente: 3. \nValor: ");
        scanf("%i", &tipo_de_usuario);

        char nome[200];
        char senha[200];

        if (tipo_de_usuario == 1) {
            status = authenticateManager(rc, db, err_msg);

        } else if (tipo_de_usuario == 2) {

        } else if (tipo_de_usuario == 3) {

        }
    } else if (autenticacao == 1) {
        printf("\nSe você e da gestão aperte: 1, Preceptor: 2 e Residente: 3. \nValor: ");
        scanf("%i", &tipo_de_usuario);

        char nome[200];
        char senha[200];
        if (tipo_de_usuario == 1) {
            printf("\nNome de usuário: ");
            scanf(" %[^\n]", nome);
            printf("Senha: ");
            scanf(" %[^\n]", senha);

            sqlite3_stmt *stmt;
            char *sql = "INSERT INTO managers(name, password) VALUES(?,?)";

            rc = sqlite3_prepare(db, sql, -1, &stmt, 0);

            if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));    
                return 1;
            }    

            sqlite3_bind_text(stmt, 1, nome, strlen(nome), NULL);
            sqlite3_bind_text(stmt, 2, senha, strlen(senha), NULL);
            rc = sqlite3_step(stmt);

            if (rc != SQLITE_DONE) {
                printf("execution failed: %s", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);    
        } else if (tipo_de_usuario == 2) {
            printf("Preceptor");
        } else if (tipo_de_usuario == 3) {
            printf("Residente");
        } else {
            printf("Sair");
        }
    }

    while(status == 1) {
        int userAction;
        
        printf("\nAções:");
        printf("\nCadastrar residentes - 1");
        printf("\nCadastrar receptores - 2");
        printf("\nSair - 3");
        printf("\nDigite o que você deseja fazer: ");

        scanf("%i", &userAction);

        if (userAction == 3) {
            status = 0;
        }
    }

    sqlite3_close(db);
    
    return 0;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;

    array[contManager] = (struct Manager *)malloc(1 * sizeof(struct Manager));

    array[contManager]->id = 1;
    strcpy(array[contManager]->name, argv[1]);
    strcpy(array[contManager]->password, argv[2]);

    contManager++;
    
    return 0;
}

int authenticateManager(int rc, sqlite3 *db, char *err_msg) {
    char nome[200];
    char senha[200];


    printf("\nNome de usuário: ");
    scanf(" %[^\n]", nome);
    printf("Senha: ");
    scanf(" %[^\n]", senha);

    char *sql = "SELECT * FROM managers;";

    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    
    int autenticado;

    for (int i = 0; i < contManager; i++) {
        if (strcmp(array[i]->name, nome) == 0 && strcmp(array[i]->password, senha) == 0) {
            autenticado = 1;
        } else {
            autenticado = 0;
        }
        // printf("ID: %i\n", array[i]->id);
        // printf("Nome: %s\n", array[i]->name);
        // printf("Senha: %s", array[i]->password);
    }

    if (autenticado == 1) {
        printf("\nCredenciais válidas.\n");
    } else {
        printf("\nCredenciais inválidas.\n");
    }

    return autenticado;
}