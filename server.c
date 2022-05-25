#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sqlite3.h>

#define PORT 2908

extern int errno;

void raspunde(int);

int logged = 0;
int logged_user = 0;
int logged_admin = 0;

int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int sd; 


    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen(sd, 20) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (1)
    {
        int client;
        int length = sizeof(from);

        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
        }
        printf("[server]S-a conectat un nou client.\n");

        if (fork() == 0)
        {
            raspunde(client);
        }
    }

    close(sd);
}

void create_account(int, char *, char *);
void login(int, char *, char *);
void register_championship(int, char*);
void logout(int);
void exit(int);
void sign_in_championship(int, char*);

void raspunde(int client)
{
    char sir[101];
    bzero(sir, 100);
    fflush(stdout);
    int lenght = 100;

    while (1)
    {
        if (read(client, sir, lenght) == -1) // se citeste comanda introdusa de client
        {
            perror("Eroare la read() de la client.\n");
            close(client);
        }

        char *com1, *com2, *com3, *com4, *com5, *com6;
        com1 = strstr(sir, "register:");
        com2 = strstr(sir, "login:");
        com3 = strstr(sir, "sign_champ:");
        com4 = strstr(sir, "reg_champ:");
        com5 = strstr(sir, "logout");
        com6 = strstr(sir, "exit");


        char copySir[100], utilizator[10];
        strcpy(copySir, sir);
        int error = 0, admin = 0, user = 0;
        char username_data[50];
        strcpy(username_data, "");
        char raspuns[200];

        if (com1 != NULL || com2 != NULL) // verific daca  au fost introduse corect comenzile register si login
        {
            char *ptr;
            ptr = strtok(copySir, ": ");
            int i = 0;
            while (ptr != NULL)
            {
                if (i == 1) //--------- register:user:name:pass se sare peste cuvantul register, se ia doar numele si parola
                {           //------- i     0     1     2    3

                    if (strcmp(ptr, "admin") == 0)  
                    { 
                        strcpy(utilizator, "admin");
                        admin = 1;
                    }
                    else if (strcmp(ptr, "user") == 0)
                    { 
                        strcpy(utilizator, "user");
                        user = 1;
                    }
                }
                else if (i == 2) // se ia numele
                {
                    strcpy(username_data, ptr);
                    strcat(username_data, ":");
                }
                else if (i == 3) // se ia parola
                {
                    strcat(username_data, ptr);
                }
                i++;
                ptr = strtok(NULL, ": ");
            }

            if (i != 4 || (admin == 0 && user == 0)) // eroare: nu s-a respectat formatul de comanda
            {
                error = 1;
                strcpy(raspuns, "[error]Comanda introdusa incorect.\n");
                if (write(client, raspuns, strlen(raspuns) + 1) == -1)
                {
                    printf("[client]Eroare la write() spre server.\n");
                }
            }
        }

        if (com1 != NULL && error == 0){
            create_account(client, username_data, utilizator);
        }
        else if (com2 != NULL && error == 0){
            login(client, username_data, utilizator);
        }

        else if (com3 != NULL){
            sign_in_championship(client, sir);
        }
        
        else if (com4 != NULL){
            register_championship(client, sir);
        }
        else if(com5 != NULL){
            logout(client);
        }
        else if(com6 != NULL){
            printf("[server]Clientul s-a deconectat de la server.\n");
        }
        else{
            char raspuns[100];
            strcpy(raspuns, "Comanda gresita.\n");

            if (write(client, raspuns, lenght) == -1)
            {
                printf("[client]Eroare la write() spre server.\n");
            }
            
        }
           
        
    }
}

void create_account(int client, char *username_data, char *utilizator)
{
    char raspuns[100];
    if (logged == 1)
    {
        strcpy(raspuns, "[error]Trebuie sa va delogati pentru a crea un cont nou.\n");
        if (write(client, raspuns, strlen(raspuns) + 1) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
    }
    else 
    {
        int userExist = 0;
        int bufferLength = 225;
        char line[bufferLength];

        char nume[50], copySir[100];
        strcpy(copySir, username_data);
        int i = 0;

        char *ptr;
        ptr = strtok(copySir, ":"); // separ nume:parola in nume si parola
        while (ptr != NULL)
        {
            if (i == 0)
                strcpy(nume, ptr);
            i++;
            ptr = strtok(NULL, ":");
        }

        FILE *filePointer;

        if (strcmp(utilizator, "user") == 0)
        {
            filePointer = fopen("users.txt", "r+");
            if (filePointer == NULL)
            {
                printf("The file does not exist!\n");
            }
        }
        else if (strcmp(utilizator, "admin") == 0)
        {
            filePointer = fopen("admin.txt", "r+");
            if (filePointer == NULL)
            {
                printf("The file does not exist!\n");
            }
        }

        while (fgets(line, bufferLength, filePointer))
        {
            char *ptr = strstr(line, nume);
            if (ptr != NULL) // s.a gasit username.ul. exista un cont cu acelasi nume
            {
                strcpy(raspuns, "[error]Exista deja un utilizator cu numele dat.\n");
                userExist = 1;
                break;
            }
        }

        if (userExist == 0)
        {
            fprintf(filePointer, "%s", username_data);
            userExist = 1;
            strcpy(raspuns, "S-a creat cu succes cont pentru utilizator dat .\n");
        }

        fclose(filePointer);

        int lenght = strlen(raspuns) + 1;

        if (write(client, raspuns, lenght) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
        else
            printf("[server]Mesajul a fost transmis cu succes.\n");
    }

    
}

void insert_championship(char* );

void login(int client, char *username_data, char *utilizator)
{
    char raspuns[100];
    if (logged == 1)
    {
        strcpy(raspuns, "[error]Un alt utilizator este deja logat.\n");
        if (write(client, raspuns, strlen(raspuns) + 1) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
    }
    else
    {
        int bufferLength = 225;
        char line[bufferLength];
        char raspuns[100];
        char nume[50], parola[50], copySir[100];
        strcpy(copySir, username_data);
        int i = 0, userExist = 0;


        char *ptr;
        ptr = strtok(copySir, ":"); // separ nume:parola primita in nume si parola
        while (ptr != NULL)
        {
            if (i == 0){
                strcpy(nume, ptr);  
                strcat(nume, ":");
            }
            else if (i == 1)
                strcpy(parola, ptr);

            i++;
            ptr = strtok(NULL, ":");
        }

        FILE *filePointer; // verific daca (,) contul dat exista si este creat  

        if (strcmp(utilizator, "user") == 0)
        {
            filePointer = fopen("users.txt", "r+");
            if (filePointer == NULL)
            {
                printf("The file does not exist!\n");
            }
        }
        else if (strcmp(utilizator, "admin") == 0)
        {
            filePointer = fopen("admin.txt", "r+");
            if (filePointer == NULL)
            {
                printf("The file does not exist!\n");
            }
        }

        while (fgets(line, bufferLength, filePointer)) 
        {
            char *ptr_nume = strstr(line, nume);
            char *ptr_parola = strstr(line, parola);

            if (ptr_nume != NULL && ptr_parola == NULL) 
            {  
                strcpy(raspuns, "[error]Utilizator gasit. Parola incorecta.\n");
                userExist = 1;
                break;
            }
            else if (ptr_nume != NULL && ptr_parola != NULL)
            {
                logged = 1;
                userExist = 1;
                strcpy(raspuns, "Utilizator logat cu succes.\n");
            }
        }

        if (logged == 1) // verific cine e logat: user sau admin . daca e user- se poate inscrie la campionat. daca e admin- inregistreaza campionat
        { 
            if (strcmp(utilizator, "user") == 0)
                logged_user = 1;
            else if (strcmp(utilizator, "admin") == 0)
                logged_admin = 1;
        }

        if (userExist == 0)
        {
            strcpy(raspuns, "[error]Nu exista cont pentru acest utilizator.\n");
        }

        fclose(filePointer);

        int lenght = strlen(raspuns) + 1;

        if (write(client, raspuns, strlen(raspuns) + 1) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
        else
            printf("[server]Mesajul a fost transmis cu succes.\n");

    }
}

void register_championship(int client,  char* sir)
{
    if(logged_admin == 0)
    {
        char raspuns[100];
        strcpy(raspuns, "[error] Doar administratorii pot inregistra un campionat .\n");
        int lenght = strlen(raspuns) + 1;

        if (write(client, raspuns, lenght) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
        else
            printf("[server]Mesajul a fost transmis cu succes.\n");
    }
    else if(logged_admin == 1)
    {
        char sql[200];
        strcpy(sql, "INSERT INTO championships (game_name, nr_players, rules, bracket_type ) values (");

        char *ptr;
        int nr_of_data = 0;
        ptr = strtok(sir, " :|"); // iau datele campionatului pentru a le trece in tabelul sql 
        while (ptr != NULL)
        {
        
            if(nr_of_data != 0){ // omite cuvantul register
                strcat(sql, "\'");
                strcat(sql, ptr);
            }

            ptr = strtok(NULL, " :|");

            if(ptr != NULL && nr_of_data != 0) // se enumera datele introduse
                strcat(sql, "\',");
            else if(ptr == NULL) 
                strcat(sql, "');");

            nr_of_data++;


        }

        // register_championship: game_name|nr_players|rules|bracket_type

        char raspuns[100];
        if(nr_of_data == 5 ) // s-a respectat formatul
        {
            insert_championship(sql);
            strcpy(raspuns, "S-a inregistrat un nou campionat.\n");
        }
        else 
        {
            strcpy(raspuns, "[error]]Respectati formatul pentru a introduce datele corect.\n");
        }

        int lenght = strlen(raspuns) + 1;

        if (write(client, raspuns, lenght) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
        else
            printf("[server]Mesajul a fost transmis cu succes.\n");
    
    }
    
    
}


void logout(int client)
{
    logged = 0;
    logged_admin = 0;
    logged_user = 0;

    char raspuns[100];

    strcpy(raspuns, "Utilizator delogat.\n");
    if (write(client, raspuns, strlen(raspuns) + 1) == -1)
    {
        printf("[client]Eroare la write() spre server.\n");
    }
    else
        printf("[server]Mesajul a fost transmis cu succes.\n");

}


void insert_championship(char* sql)
{
    sqlite3 *db;
    int rett = sqlite3_open("championships.db", &db);
    if (rett != SQLITE_OK)
    {
        printf("Can t open database\n");
        sqlite3_close(db);
    }
    else
    {
        printf("Opened database successufully\n");
        sqlite3_exec(db, sql, 0, 0, NULL);
        printf("S-a inregistrat un nou campionat.\n");
    }
    
}

int max_nr_players_global;
int signed_nr_players_global;
char info[200];

int Callback(void *pArg, int count, char **data, char **columnNames) 
{ 
    int idx;
    for (idx = 0; idx < count; idx++)
    {
        max_nr_players_global = atoi(data[idx]);
    }

    return 0;
}

int verify_nr_players(char* game_name)
{
    char sql[200];
    strcpy(sql, "SELECT nr_players FROM championships where game_name = ");
    strcat(sql, "\"");
    strcat(sql, game_name);
    strcat(sql, "\"");

    sqlite3 *db;
    int rett = sqlite3_open("championships.db", &db);
    if (rett != SQLITE_OK)
    {
        printf("Can t open database\n");
        sqlite3_close(db);
    }
    else
    {
        sqlite3_exec(db, sql, Callback, 0, NULL);
    }

    return max_nr_players_global;
}

int Callback2(void *pArg, int count, char **data, char **columnNames) 
{ 
  int idx;
  for (idx = 0; idx < count; idx++)
  {
    signed_nr_players_global = atoi(data[idx]);
  }
  
  return 0;
}

int verify_signed_nr_players(char* game_name)
{
    char sql[200];
    strcpy(sql, "SELECT COUNT (*) FROM inscrieri where game_name = ");
    strcat(sql, "\"");
    strcat(sql, game_name);
    strcat(sql, "\"");

    sqlite3 *db;
    int rett = sqlite3_open("championships.db", &db);
    if (rett != SQLITE_OK)
    {
        printf("Can t open database\n");
        sqlite3_close(db);
    }
    else
    {
        sqlite3_exec(db, sql, Callback2, 0, NULL);
    }

    return signed_nr_players_global;
    
}

void sign_in_database_inscrieri(char* sql)
{
    sqlite3 *db;
    int rett = sqlite3_open("championships.db", &db);
    if (rett != SQLITE_OK)
    {
        printf("Can t open database\n");
        sqlite3_close(db);
    }
    else
    {
        printf("Opened database successufully\n");
        sqlite3_exec(db, sql, 0, 0, NULL);
        printf("S-a facut o noua inscriere in campionate.\n");
    }
    
}


void sign_in_championship(int client, char* sir)
{
    // se verifica daca s-a scris comanda corect --> sign_champ: game_name|user_name|score|email_address
    
    if(logged_user == 0)
    {
        char raspuns[100];
        strcpy(raspuns, "[error] Doar utilizatorii se pot inscrie in campionate.\n");
        int lenght = strlen(raspuns) + 1;

        if (write(client, raspuns, lenght) == -1)
        {
            printf("[client]Eroare la write() spre server.\n");
        }
        else
            printf("[server]Mesajul a fost transmis cu succes.\n");
    }
    else if(logged_user == 1) 
    {
        //-------------------------------------------------- sign_champ: game_name|user_name|score|email_address
        // split command into words               nr_of_data:    0           1         2       3        4

        char game_name[50];
        char user_name[50];
        char score[50];
        char email_address[50];
        char raspuns[200];

        int nr_of_data = 0;
        char *ptr;
        ptr = strtok(sir, " :|");

        while(ptr != NULL)
        {
            if(nr_of_data == 1){
                strcpy(game_name, ptr);
            }
            else if(nr_of_data == 2){
                strcpy(user_name, ptr);
            }
            else if(nr_of_data == 3){
                strcpy(score, ptr);
            }
            else if(nr_of_data == 4){
                strcpy(email_address, ptr);
            }

            nr_of_data++;
            ptr = strtok(NULL, " :|");
        }
        
        int error = 1;

        if(nr_of_data == 5) // s.a respectat formatul
            error = 0;

        //--------------------------------------------------

        // se verifica daca mai sunt locuri in campionat 

        if(error == 0)
        {
            
            int inscris = 0;

            int max_nr_players = verify_nr_players(game_name); // iau numarul maxim de jucatori
            int signed_nr_players = verify_signed_nr_players(game_name);

            if(signed_nr_players < max_nr_players){
                // se poate inscrie

                char sql[200];
                strcpy(sql, "INSERT INTO inscrieri (game_name, user_name, score, email_address) values ("); // 'game_name','user_name','score','email_address');
                strcat(sql, "\'");
                strcat(sql, game_name);
                strcat(sql, "\',\'");
                strcat(sql, user_name);
                strcat(sql, "\',\'");
                strcat(sql, score);
                strcat(sql, "\',\'");
                strcat(sql, email_address);
                strcat(sql, "\');");

                sign_in_database_inscrieri(sql);
                inscris = 1;

                strcpy(raspuns, "V-ati inscris cu succes in campionat.\n");

            }
            else if (signed_nr_players == max_nr_players){
                
                strcpy(raspuns, "[error]Nu mai exista locuri libere in campionat. Inscrierea a esuat.\n");
            }

            int lenght = strlen(raspuns) + 1;

            if (write(client, raspuns, lenght) == -1)
            {
                printf("[client]Eroare la write() spre server.\n");
            }
            else
                printf("[server]Mesajul a fost transmis cu succes.\n");


        }
        else{
            strcpy(raspuns, "[error] Comanda introdusa incorect.\n");

            int lenght = strlen(raspuns) + 1;

            if (write(client, raspuns, lenght) == -1)
            {
                printf("[client]Eroare la write() spre server.\n");
            }
            else
                printf("[server]Mesajul a fost transmis cu succes.\n");
        }
        

    }


}

