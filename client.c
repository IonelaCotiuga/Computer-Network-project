#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sqlite3.h>

extern int errno;

int port;

void show_championships();

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  char sir[200];             // mesajul trimis
  char sir_recv[200];

  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }
  port = atoi(argv[2]);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  server.sin_family = AF_INET;

  server.sin_addr.s_addr = inet_addr(argv[1]);

  server.sin_port = htons(port);

  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  while (1)
  {
    bzero(sir, 200);

    printf("register user/admin       --> register:user:name:password / register:admin:name:password \n"); 
    printf("login user / admin        --> login:user:name:password    / login:admin:name:password \n");
    printf("logout                    --> logout \n");
    printf("register championship     --> reg_champ: game_name|nr_players|rules|bracket_type\n");
    printf("sign in to a championship --> sign_champ: game_name|user_name|score|email_address\n");
    printf("\n");

    
    printf("[client]Introduceti comanda: ");
    fflush(stdout);
    read(0, sir, 200);

    if(strstr(sir, "exit") != NULL){
      close(sd);
      printf("Client deconectat de la server.\n");
      exit(1);
    }
    else
    {
        if (write(sd, sir, strlen(sir) + 1) == -1)
        {
          printf("[client]Eroare la write() spre server.\n");
          return errno;
        }

        if (read(sd, sir_recv, 200) < 0)
        {
          perror("[client]Eroare la read() de la server.\n");
          return errno;
        }

        // afisam mesajul primit
        printf("\n");
        printf("[client]Mesajul primit este: %s\n", sir_recv);

        if ((strstr(sir, "login:user") != NULL || strstr(sir, "reg_champ") != NULL) && strstr(sir_recv, "error") == NULL)
        {
          printf("Lista campionate: \n");
          printf("\n");
          show_championships();
        }
    }
    
  }

  // inchidem conexiunea
  close(sd);
}

int Callback(void *pArg, int count, char **data, char **columnNames)
{ 
  int idx;

  for (idx = 0; idx < count; idx++)
  {
    printf(" %s : %s\n", columnNames[idx], data[idx]);
  }
  printf("\n");

  return 0;
}

void show_championships()
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
    sqlite3_exec(db, "SELECT * FROM championships", Callback, 0, NULL);
  }
}