// gcc -o main main.c sqlite3.c sqlite3.h
// login: preceptor / senha: 123

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int callbackManager(void *, int, char **, char **);
int callbackPreceptor(void *, int, char **, char **);
int callbackResidencies(void *NotUsed, int argc, char **argv, char **azColName);

int authenticateManager(int rc, sqlite3 *db, char *err_msg);
int authenticatePreceptor(int rc, sqlite3 *db, char *err_msg);
int createResidency(int rc, sqlite3 *db, char *err_msg);

struct Manager{
    int id;
    char name[200];
    char password[200];
};

struct Resident{
    int id;
    char name[200];
    char password[200];
    char preceptor_id[200];
    char residency_id[200];
};

struct Residency{
    int id;
    char residencyName[20];
};

struct Preceptor{
    int id;
    char name[200];
    char password[200];
};

struct Manager *managers[50];
struct Resident *residents[50];
struct Preceptor *preceptors[50];
struct Residency *residencies[50];
int contManager = 0;
int contResident = 0;
int contPreceptor = 0;
int contResidency = 0;

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
                "CREATE TABLE IF NOT EXISTS preceptors(id INTEGER PRIMARY KEY, name TEXT, password TEXT, residency_id INTEGER);"
                "CREATE TABLE IF NOT EXISTS residents(id INTEGER PRIMARY KEY, name TEXT, password TEXT, preceptor_id INTEGER, residency_id INTEGER);"
                "CREATE TABLE IF NOT EXISTS residencies(id INTEGER PRIMARY KEY, residencyName TEXT);"
                "CREATE TABLE IF NOT EXISTS activities(id INTEGER PRIMARY KEY, activityName TEXT, description TEXT, max_grade INTEGER);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        return 1;
    } 

    printf("Faca seu login!\nDigite 1 para fazer login e 2 para criar uma conta. \nValor: ");
    scanf("%i", &autenticacao);

    int status = 0;

    if (autenticacao == 1) {
        printf("\nSe você e da gestão digite: 1, Preceptor: 2 e Residente: 3. \nValor: ");
        scanf("%i", &tipo_de_usuario);

        char nome[200];
        char senha[200];

        if (tipo_de_usuario == 1) {
            status = authenticateManager(rc, db, err_msg);

        } else if (tipo_de_usuario == 2) {
            status = authenticatePreceptor(rc, db, err_msg);

        } else if (tipo_de_usuario == 3) {

        }
        // CADASTRA O NOVO USUÁRIO CASO A OPÇÃO FOR 2
    } else if (autenticacao == 2) {
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

    // ENTRA NAS AÇÕES DO MANAGER (GESTÃO) SE A FUNÇÃO AUTHENTICATEMANAGER RETORNAR 1
    while(status == 1) {
        int userAction;
        
        printf("\n---- Ações da Gestão ----\n");
        printf("\nCriar residência - 1");
        printf("\nCadastrar residentes - 2");
        printf("\nCadastrar preceptores - 3");
        printf("\nSair - 4");
        printf("\nDigite o que você deseja fazer: ");

        scanf("%i", &userAction);

        // CASO 1, CHAMA FUNÇÃO PARA CRIAR RESIDENCIA
        if (userAction == 1) {
            createResidency(rc, db, err_msg);
        }

        // CASO 2, CADASTRA RESIDENTE
        if (userAction == 2) {
            char nome[200];
            char senha[200];
            int preceptorId;
            int residency_id;

            char *sqlPreceptors = "SELECT * FROM preceptors;";
            rc = sqlite3_exec(db, sqlPreceptors, callbackPreceptor, 0, &err_msg);

            char *sqlResidencies = "SELECT * FROM residencies";
            rc = sqlite3_exec(db, sqlResidencies, callbackResidencies, 0, &err_msg);

            printf("\n---- Cadastre o residente ----\n");

            printf("Nome de usuário: ");
            scanf(" %[^\n]", nome);
            printf("Senha: ");
            scanf(" %[^\n]", senha);
            printf("\nLista dos preceptores: \n\n");

            for (int i = 0; i < contPreceptor; i++) {
                printf("ID: %i -- Nome: %s\n", preceptors[i]->id, preceptors[i]->name);
            }

            printf("\nEscolha o ID do Preceptor: ");
            scanf("%i", &preceptorId);

            printf("\nLista das Residências: \n\n");

            for(int i=0;i<contResidency;i++){
                printf("ID: %i -- Nome: %s\n", residencies[i]->id, residencies[i]->residencyName);
            }

            printf("\nEscolha o ID da Residência: ");
            scanf("%i", &residency_id);

            sqlite3_stmt *stmt;
            char *sql = "INSERT INTO residents(name, password, preceptor_id, residency_id) VALUES(?,?,?,?)";

            rc = sqlite3_prepare(db, sql, -1, &stmt, 0);

            if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));    
                return 1;
            }    

            sqlite3_bind_text(stmt, 1, nome, strlen(nome), NULL);
            sqlite3_bind_text(stmt, 2, senha, strlen(senha), NULL);
            sqlite3_bind_int(stmt, 3, preceptorId);
            sqlite3_bind_int(stmt, 4, residency_id);

            rc = sqlite3_step(stmt);

            if (rc != SQLITE_DONE) {
                printf("execution failed: %s", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);   
        }
  
        // CASO 3, CADASTRA PRECEPTORES
        if (userAction == 3) {
            char nome[200];
            char senha[200];
            
            printf("\n---- Cadastre o Preceptor ----\n");

            printf("Nome de usuário: ");
            scanf(" %[^\n]", nome);
            printf("Senha: ");
            scanf(" %[^\n]", senha);
            
            sqlite3_stmt *stmt;
            char *sql = "INSERT INTO preceptors(name, password) VALUES(?,?)";

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
        }

        // CASO 4, DESLOGA
        if (userAction == 4) {
            status = 0;
        }
    }

    while(status == 2) {
        int userAction;

        printf("\n---- Ações do Preceptor ----\n");
        printf("\nVer Meus Residentes - 1\n");
        scanf("%i", &userAction);
    }
    sqlite3_close(db);
    
    return 0;
}

int callbackManager(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;

    managers[contManager] = (struct Manager *)malloc(1 * sizeof(struct Manager));

    managers[contManager]->id = 1;
    strcpy(managers[contManager]->name, argv[1]);
    strcpy(managers[contManager]->password, argv[2]);

    contManager++;
    
    return 0;
}

int callbackResident(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;

    residents[contResident] = (struct Resident *)malloc(1 * sizeof(struct Resident));

    residents[contResident]->id = 1;
    strcpy(residents[contResident]->name, argv[1]);
    strcpy(residents[contResident]->password, argv[2]);
    strcpy(residents[contResident]->preceptor_id, argv[3]);

    contResident++;
    
    return 0;
}

int callbackResidencies(void *NotUsed, int argc, char **argv, char **azColName){
    NotUsed = 0;

    residencies[contResidency] = (struct Residency *)malloc(1 * sizeof(struct Residency));

    residencies[contResidency]->id = contResidency;
    strcpy(residencies[contResidency]->residencyName, argv[1]);

    contResidency++;

    return 0;
}

int callbackPreceptor(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;

    preceptors[contPreceptor] = (struct Preceptor *)malloc(1 * sizeof(struct Preceptor));

    preceptors[contPreceptor]->id = contPreceptor;
    strcpy(preceptors[contPreceptor]->name, argv[1]);
    strcpy(preceptors[contPreceptor]->password, argv[2]);

    contPreceptor++;
    
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

    rc = sqlite3_exec(db, sql, callbackManager, 0, &err_msg);
    
    int autenticado;

    for (int i = 0; i < contManager; i++) {
        if (strcmp(managers[i]->name, nome) == 0 && strcmp(managers[i]->password, senha) == 0) {
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

int authenticatePreceptor(int rc, sqlite3 *db,char *err_msg){
    char nome[200];
    char senha[200];

    printf("\nNome do usuário: ");
    scanf(" %[^\n]", nome);
    printf("Senha: ");
    scanf(" %[^\n]", senha);

    char *sql = "SELECT * FROM preceptors;";

    rc = sqlite3_exec(db, sql, callbackPreceptor, 0, &err_msg);

    int autenticado;

    for(int i = 0; i < contPreceptor; i++){
        if(strcmp(preceptors[i]->name, nome) == 0 && strcmp(preceptors[i]->password, senha) == 0){
            autenticado = 2;
        }else{
            autenticado = 0;
        }
    }

    if(autenticado == 2){
        printf("\nCredenciais válidas.\n");
    }else{
        printf("\nCredenciais inválidas.\n");
    }

    return autenticado;
}

int createResidency(int rc, sqlite3 *db, char *err_msg) {
    char nome[200];

    printf("\n---- Cadastre uma residência ----\n");

    printf("Nome da residência: ");
    scanf(" %[^\n]", nome);

    sqlite3_stmt *stmt;
    char *sql = "INSERT INTO residencies(residencyName) VALUES(?)";

    rc = sqlite3_prepare(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));    
        return 1;
    }    

    sqlite3_bind_text(stmt, 1, nome, strlen(nome), NULL);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);   

    printf("\nResidência criada com sucesso.\n");

    contResidency++;

    return 1;
}


//status = authenticatePreceptor(rx, db, err_msg);