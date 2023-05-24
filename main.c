// gcc -o main main.c sqlite3.c sqlite3.h
// login: preceptor / senha: 123

#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int callbackManager(void *NotUsed, int argc, char **argv, char **azColName);
int callbackPreceptor(void *NotUsed, int argc, char **argv, char **azColName);
int callbackResidencies(void *NotUsed, int argc, char **argv, char **azColName);

int authenticateManager(int rc, sqlite3 *db, char *err_msg);
int authenticatePreceptor(int rc, sqlite3 *db, char *err_msg);
int authenticateResident(int rc, sqlite3 *db, char *err_msg);

int createResidency(int rc, sqlite3 *db, char *err_msg);
int createActivity(int rc, sqlite3 *db, char *err_msg);

int visualizeResidencyActivities(int rc, sqlite3 *db, char *err_msg);
int visualizeAndMarkActivities(int rc, sqlite3 *db, char *err_msg, char id[10]);

int markFrequency(int rc, sqlite3 *db, char *err_msg, char id[10]);
void viewResidentData(int *rc, sqlite3 *db, char preceptor_id[10]);

struct Manager{
    int id;
    char name[200];
    char password[200];
};

struct Resident{
    char id[10];
    char name[200];
    char password[200];
    char preceptor_id[200];
    char residency_id[200];
    char frequency[10];
};

struct Residency{
    char id[10];
    char residencyName[20];
    char residency_course[30];
};

struct Preceptor{
    int id;
    char charId[10];
    char name[200];
    char password[200];
    int residency_id;
};

struct Activity{
    int id;
    char name[200];
    char description[1000];
    int max_grade;
};

struct Manager *managers[50];
struct Resident *residents[50];
struct Preceptor *preceptors[50];
struct Residency *residencies[50];
int contManager = 0;
int contResident = 0;
int contPreceptor = 0;
int contResidency = 0;
char currentUserID[10];
char currentPreceptorID[10];

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
                "CREATE TABLE IF NOT EXISTS residents(id INTEGER PRIMARY KEY, name TEXT, password TEXT, preceptor_id INTEGER, residency_id INTEGER, frequency INTEGER DEFAULT 0);"
                "CREATE TABLE IF NOT EXISTS residencies(id INTEGER PRIMARY KEY, residencyName TEXT, residency_course TEXT);"
                "CREATE TABLE IF NOT EXISTS activities(id INTEGER PRIMARY KEY, name TEXT, description TEXT, max_grade INTEGER, residency_id INTEGER REFERENCES residencies(id));"
                "CREATE TABLE IF NOT EXISTS activities_residents(id INTEGER PRIMARY KEY, activity_id INTEGER, residency_id INTEGER, user_id INTEGER, done INTEGER, grade INTEGER);";

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
            status = authenticateResident(rc, db, err_msg);
        }
        // CADASTRA O NOVO USUÁRIO CASO A OPÇÃO FOR 2
    } else if (autenticacao == 2) {
        char nome[200];
        char senha[200];
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
    }

    // ENTRA NAS AÇÕES DO MANAGER
    while(status == 1) {
        int userAction;
        
        printf("\n---- Ações da Gestão ----\n");
        printf("\nCriar residência - 1");
        printf("\nCadastrar residentes - 2");
        printf("\nCadastrar preceptores - 3");
        printf("\nSair - 4");
        printf("\nCadastrar Atividades - 5");
        printf("\nVisualizar Atividades preceptores - 6");
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
            int sum=0;
            int found=0;
            int collectPreceptors[100];

            char *sqlPreceptors = "SELECT * FROM preceptors;";

            rc = sqlite3_exec(db, sqlPreceptors, callbackPreceptor, 0, &err_msg);

            char *sqlResidencies = "SELECT * FROM residencies";
            rc = sqlite3_exec(db, sqlResidencies, callbackResidencies, 0, &err_msg);

            printf("\n---- Cadastre o residente ----\n");

            printf("Nome de usuário: ");
            scanf(" %[^\n]", nome);
            printf("Senha: ");
            scanf(" %[^\n]", senha);

            
            printf("\nLista das Residências: \n\n");

            for(int i=0;i<contResidency;i++){
                printf("ID: %s -- Nome: %s\n", residencies[i]->id, residencies[i]->residencyName);
            }

            printf("\nEscolha o ID da Residência: ");
            scanf("%i", &residency_id);

            while(residency_id > contResidency || residency_id < 0){
                printf("Residência não encontrada, por favor, digite novamente.\n");
                printf("Escolha o ID da Residência: ");
                scanf("%i",&residency_id);
                printf("\n");
            }

            printf("\nPreceptores da residência ID: %ls\n",&residency_id);

            for (int i = 0; i < contPreceptor; i++) {
                if(preceptors[i]->id == residency_id){
                    printf("ID: %i -- Nome: %s\n", preceptors[i]->id, preceptors[i]->name);
                    collectPreceptors[sum] = preceptors[i]->id;
                    sum++;
                } 
            }

            printf("\nEscolha o ID do Preceptor: ");
            scanf("%i", &preceptorId);

            while(!found){

                printf("\nEscolha o ID do Preceptor: ");
                scanf("%i", &preceptorId);

                for (int i = 0; i < sum; i++) {
                    if (preceptorId == collectPreceptors[i]) {
                    found = 1;
                    break;
                    }
                }

                if (!found) {
                     printf("O Preceptor digitado não é válida,tente novamente. \n");
                }
                
            }

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

            printf("\nResidente cadastrado com sucesso.\n");
        }
        // CASO 3, CADASTRA PRECEPTORES
        if (userAction == 3) {
            char nome[200];
            char senha[200];
            int residency_id;

            char *sqlResidencies = "SELECT * FROM residencies";
            rc = sqlite3_exec(db, sqlResidencies, callbackResidencies, 0, &err_msg);
            
            printf("\n---- Cadastre o Preceptor ----\n");

            printf("Nome de usuário: ");
            scanf(" %[^\n]", nome);
            printf("Senha: ");
            scanf(" %[^\n]", senha);

            printf("\nLista das Residências: \n\n");

            for(int i=0;i<contResidency;i++){
                printf("ID: %s -- Nome: %s\n", residencies[i]->id, residencies[i]->residencyName);
            }

            printf("\nEscolha o ID da Residência: ");
            scanf("%i", &residency_id);

            while(residency_id > contResidency || residency_id < 0){
                printf("Residência não encontrada, por favor, digite novamente.\n");
                printf("Escolha o ID da Residência: ");
                scanf("%i",&residency_id);
                printf("\n");
            }

            
            sqlite3_stmt *stmt;
            char *sql = "INSERT INTO preceptors(name, password, residency_id) VALUES(?,?,?)";

            rc = sqlite3_prepare(db, sql, -1, &stmt, 0);

            if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));    
                return 1;
            }    

            sqlite3_bind_text(stmt, 1, nome, strlen(nome), NULL);
            sqlite3_bind_text(stmt, 2, senha, strlen(senha), NULL);
            sqlite3_bind_int(stmt, 3, residency_id);
            rc = sqlite3_step(stmt);

            if (rc != SQLITE_DONE) {
                printf("execution failed: %s", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);  

            printf("\nPreceptor cadastrado com sucesso.\n"); 
        }
        // CASO 4, DESLOGA
        if (userAction == 4) {
            status = 0;
        }
        //CASO 5, CADASTRA AS ATIVIDADES
        if(userAction == 5) {
            createActivity(rc, db, err_msg);
        }
        //CASO 6, LISTA AS ATIVIDADES
        if(userAction == 6) {
            visualizeResidencyActivities(rc, db, err_msg);
        }
    }
    // ENTRA NAS AÇÕES DO PRECEPTOR
    while(status == 2) {
        int userAction;
                
        printf("\n---- Ações do Preceptor ----\n");
        printf("\nVisualizar residentes - 1");
        printf("\nSair - 4");
        printf("\nDigite o que você deseja fazer: ");

        scanf("%i", &userAction);

        if (userAction == 1) {
            //print preceptor ID
            printf("\nID do Preceptor: %s\n", currentPreceptorID);

            char *sqlResidents = "SELECT * FROM residents WHERE preceptor_id = ?";
            sqlite3_stmt *residents_stmt;

            rc = sqlite3_prepare_v2(db, sqlResidents, -1, &residents_stmt, 0);

            if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
                return 1;
            }

            sqlite3_bind_text(residents_stmt, 1, currentPreceptorID, -1, SQLITE_STATIC);

            viewResidentData(&rc, db, currentPreceptorID);

            sqlite3_finalize(residents_stmt);
        }
    }
    // ENTRA NAS AÇÕES DO RESIDENTE
    while(status == 3) {
        int userAction;
        int residencyID;
                
        printf("\n---- Ações do Residente ----\n");
        printf("\nRegistrar Atividade Realizada - 1");
        printf("\nRegistrar Frequência - 2");
        printf("\nSair - 4");
        printf("\nDigite o que você deseja fazer: ");

        scanf("%i", &userAction);


        if (userAction == 1){
            for(int i=0; i<contResident; i++){
                // if(residents[i]->id == currentUserID){
                if(strcmp(residents[i]->id, currentUserID) == 0){
                    printf("Olá %s, informe as atividades realizadas hoje:", residents[i]->name);
                    // residencyID = (int)residents[i]->residency_id;

                    // printf("ResidencyID de %s: %s", residents[i]->name, residents[i]->residency_id);
                    visualizeAndMarkActivities(rc, db, err_msg, residents[i]->residency_id);
                }
            }
        }
        // REGISTRA FREQUENCIA
        if (userAction == 2){
            markFrequency(rc, db, err_msg, currentUserID);
        }
        if (userAction == 4) {
            status = 0;
        }

    }
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

    // char c = argv[0];
    // char* ponteiroChar = &c;
    // void* ponteiroVoid = ponteiroChar;
    // int* ponteiroInt = ponteiroVoid;

    residents[contResident] = (struct Resident *)malloc(1 * sizeof(struct Resident));

    // residents[contResident]->id = contResident;
    strcpy(residents[contResident]->id, argv[0]);
    strcpy(residents[contResident]->name, argv[1]);
    strcpy(residents[contResident]->password, argv[2]);
    strcpy(residents[contResident]->preceptor_id, argv[3]);
    strcpy(residents[contResident]->residency_id, argv[4]);
    strcpy(residents[contResident]->frequency, argv[5]);

    contResident++;
    
    return 0;
}

int callbackResidencies(void *NotUsed, int argc, char **argv, char **azColName){
    NotUsed = 0;

    residencies[contResidency] = (struct Residency *)malloc(1 * sizeof(struct Residency));

    strcpy(residencies[contResidency]->id, argv[0]);
    // residencies[contResidency]->id = contResidency;
    strcpy(residencies[contResidency]->residencyName, argv[1]);
    strcpy(residencies[contResidency]->residency_course, argv[2]);

    contResidency++;

    return 0;
}

int callbackPreceptor(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;

    preceptors[contPreceptor] = (struct Preceptor *)malloc(1 * sizeof(struct Preceptor));

    preceptors[contPreceptor]->id = contPreceptor;
    strcpy(preceptors[contPreceptor]->charId, argv[0]);
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
            if(autenticado == 1) {
                break;
            }
        } else {
            autenticado = 0;
        }
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
            if(autenticado == 2) {
                strcpy(currentPreceptorID, preceptors[i]->charId);
                break;
            }
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
    char curso_residencia[200];

    printf("\n---- Cadastre uma residência ----\n");

    printf("Nome da residência: ");
    scanf(" %[^\n]", nome);

    printf("Curso da residência: ");
    scanf(" %[^\n]", curso_residencia);

    sqlite3_stmt *stmt;
    char *sql = "INSERT INTO residencies(residencyName, residency_course) VALUES(?,?)";

    rc = sqlite3_prepare(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));    
        return 1;
    }    

    sqlite3_bind_text(stmt, 1, nome, strlen(nome), NULL);
    sqlite3_bind_text(stmt, 2, curso_residencia, strlen(curso_residencia), NULL);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);   

    printf("\nResidência criada com sucesso.\n");

    contResidency++;

    return 1;
}

int createActivity(int rc, sqlite3 *db, char *err_msg) {
    char name[200];
    char description[1000];
    int max_grade = 0;
    int residency_id;

    printf("\n---- Cadastre uma Atividade ----\n");

    printf("Nome da Atividade: ");
    scanf(" %[^\n]", name);

    printf("Descrição da Atividade: ");
    scanf(" %[^\n]", description);

    printf("Nota Máxima da Atividade: ");
    scanf("%d", &max_grade);

    char *sqlResidencies = "SELECT * FROM residencies";
    sqlite3_stmt *residency_stmt;

    rc = sqlite3_prepare_v2(db, sqlResidencies, -1, &residency_stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    printf("\nLista das Residências: \n\n");

    while ((rc = sqlite3_step(residency_stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(residency_stmt, 0);
        const unsigned char *name = sqlite3_column_text(residency_stmt, 1);
        printf("ID: %i -- Nome: %s\n", id, name);
    }

    sqlite3_finalize(residency_stmt);
    printf("\nEscolha o ID da Residência: ");
    scanf("%d", &residency_id);

    sqlite3_stmt *stmt;
    char *sql = "INSERT INTO activities(name, description, max_grade, residency_id) VALUES(?, ?, ?, ?)";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_text(stmt, 1, name, strlen(name), NULL);
    sqlite3_bind_text(stmt, 2, description, strlen(description), NULL);
    sqlite3_bind_int(stmt, 3, max_grade);
    sqlite3_bind_int(stmt, 4, residency_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    printf("\nAtividade criada com sucesso.\n");

    return 1;
}

int visualizeResidencyActivities(int rc, sqlite3 *db, char *err_msg) {
    int residency_id;

    char *sqlResidencies = "SELECT * FROM residencies";
    sqlite3_stmt *residency_stmt;

    rc = sqlite3_prepare_v2(db, sqlResidencies, -1, &residency_stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    printf("\nLista das Residências: \n\n");

    while ((rc = sqlite3_step(residency_stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(residency_stmt, 0);
        const unsigned char *name = sqlite3_column_text(residency_stmt, 1);
        printf("ID: %i -- Nome: %s\n", id, name);
    }

    sqlite3_finalize(residency_stmt);

    printf("\nEscolha o ID da Residência: ");
    scanf("%d", &residency_id);

    char *sqlActivities = "SELECT * FROM activities WHERE residency_id = ?";
    sqlite3_stmt *activity_stmt;

    rc = sqlite3_prepare_v2(db, sqlActivities, -1, &activity_stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(activity_stmt, 1, residency_id);

    printf("\nAtividades da Residência Escolhida: \n\n");

    while ((rc = sqlite3_step(activity_stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(activity_stmt, 0);
        const unsigned char *name = sqlite3_column_text(activity_stmt, 1);
        const unsigned char *description = sqlite3_column_text(activity_stmt, 2);
        int max_grade = sqlite3_column_int(activity_stmt, 3);
        printf("ID: %i -- Nome: %s -- Descrição: %s -- Nota Máxima: %d\n", id, name, description, max_grade);
    }

    sqlite3_finalize(activity_stmt);

    return 1;
}

int authenticateResident(int rc, sqlite3 *db, char *err_msg){
    char nome[200];
    char senha[200];

    printf("\nNome do usuário: ");
    scanf(" %[^\n]", nome);
    printf("Senha: ");
    scanf(" %[^\n]", senha);

    char *sql = "SELECT * FROM residents;";

    rc = sqlite3_exec(db, sql, callbackResident, 0, &err_msg);

    int autenticado;

    for(int i = 0; i< contResident; i++){
        if(strcmp(residents[i]->name, nome) == 0 && strcmp(residents[i]->password, senha) == 0){
            autenticado = 3;
            if(autenticado == 3){
                strcpy(currentUserID, residents[i]->id);
                // currentUserID = residents[i]->id;
                break;
            }
        }else{
            autenticado = 0;
        }
    }
    if(autenticado == 3){
        printf("\nCredenciais Válidas.\n");
    }else{
        printf("\nCredenciais inválidas.\n");
    }
    return autenticado;
}

int visualizeAndMarkActivities(int rc, sqlite3 *db, char *err_msg, char id[10]){
    //VISUALIZAR
    
    char *sqlActivities = "SELECT * FROM activities WHERE cast(residency_id AS INTEGER) = ?";
    sqlite3_stmt *activity_stmt;

    rc = sqlite3_prepare_v2(db, sqlActivities, -1, &activity_stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // sqlite3_bind_int(activity_stmt, 1, id);
    sqlite3_bind_text(activity_stmt, 1, id, strlen(id), NULL);

    printf("\nAtividades da Residência: \n\n");

    while ((rc = sqlite3_step(activity_stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(activity_stmt, 0);
        const unsigned char *name = sqlite3_column_text(activity_stmt, 1);
        const unsigned char *description = sqlite3_column_text(activity_stmt, 2);
        int max_grade = sqlite3_column_int(activity_stmt, 3);
        printf("ID: %i -- Nome: %s -- Descrição: %s -- Nota Máxima: %d\n", id, name, description, max_grade);

        // currentUserID
        // activityID
        // residencyID
        // done -> 1
    }
    sqlite3_finalize(activity_stmt);
    
    sqlite3_stmt *stmt;
    char *sql = "INSERT INTO activities_residents(activity_id, residency_id, user_id, done) VALUES(?,?,?,1)";

    rc = sqlite3_prepare(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));    
        return 1;
    }    

    int activityId;
    int residencyId;
    char userId[10];

    scanf("Informe o ID da atividade que quer Registrar", &activityId);
    strcmp(userId, currentUserID);

    sqlite3_bind_int(stmt, 1, activityId);
    sqlite3_bind_int(stmt, 2, residencyId);
    sqlite3_bind_text(stmt, 3, userId, strlen(userId), NULL);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return 1;
}

int markFrequency(int rc, sqlite3 *db, char *err_msg, char id[10]) {
    sqlite3_stmt *update_stmt;

    char *sql = "UPDATE residents SET frequency = frequency + 1 WHERE cast(id AS INTEGER) = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &update_stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    rc = sqlite3_bind_text(update_stmt, 1, id, strlen(id), NULL);

    if (rc != SQLITE_DONE) {
        printf("error: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(update_stmt);

    if (rc != SQLITE_DONE) {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(update_stmt); 

    printf("\nFrequencia Realizada com Sucesso!\n");

    return 1;
}

void viewResidentData(int *rc, sqlite3 *db, char preceptor_id[10]) {
    sqlite3_stmt *stmt;

    // Listar residentes do preceptor
    char *sqlListResidents = "SELECT * FROM residents WHERE preceptor_id = ?";
    *rc = sqlite3_prepare_v2(db, sqlListResidents, -1, &stmt, 0);

    if (*rc != SQLITE_OK) {
        printf("Não foi possível preparar a declaração: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, preceptor_id, -1, SQLITE_STATIC);

    printf("\nResidents:\n");
    while ((*rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("ID: %s\n", sqlite3_column_text(stmt, 0));
        printf("Nome: %s\n", sqlite3_column_text(stmt, 1));
        printf("Frequência: %s\n", sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);

    // Listar atividades já feitas pelos residentes
    printf("\nCompleted Activities of Residents:\n");
    char *sqlListCompletedActivities = "SELECT residents.name AS residentName, activities.name AS activityName, activities.description AS activityDescription "
                                       "FROM residents "
                                       "INNER JOIN activities_residents ON residents.id = activities_residents.user_id "
                                       "INNER JOIN activities ON activities_residents.activity_id = activities.id "
                                       "WHERE residents.preceptor_id = ? AND activities_residents.done = 1";
    *rc = sqlite3_prepare_v2(db, sqlListCompletedActivities, -1, &stmt, 0);

    if (*rc != SQLITE_OK) {
        printf("Não foi possível preparar a declaração: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, preceptor_id, -1, SQLITE_STATIC);

    while ((*rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("Residente: %s\n", sqlite3_column_text(stmt, 0));
        printf("Atividade: %s\n", sqlite3_column_text(stmt, 1));
        printf("Descrição: %s\n", sqlite3_column_text(stmt, 2));
    }

    sqlite3_finalize(stmt);
}

//status = authenticatePreceptor(rx, db, err_msg);

// Activities
// id
// name
// description
// max_grade
// residency_id

// Residents
// id
// name
// password
// preceptor_id
// residency_id
// frequency

// atividade - residente
// activity_id
// residency_id
// user_id
// done
// grade

// FLUXO DE RESIDENTE
// MARCAR FREQUÊNCIA DE ATIVIDADE -> insert into activities_residents VALUES (activity_id, user_id, done, grade) (id, id, 1, NULL)
// VISUALIZAR MINHAS ATIVIDADES -> select * from activites_residents where user_id = ? and done == 1

// FLUXO DO PRECEPTOR
// LISTAR RESIDENTES DO PRECEPTOR -> select * from residents where preceptor_id == PRECEPTOR_ID;
// LISTAR ATIVIDADES DO RESIDENTE ESPECÍFICO: select * from activities_residents WHERE user_id == USER_ID_ESCOLHIDO
// ESCOLHER ATIVIDADE DO RESIDENTE: select * from activies where id == activity_id
// DAR NOTA: insert into tabela (grade) values (nota do preceptor)


