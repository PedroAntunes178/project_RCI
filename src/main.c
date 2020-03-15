#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "server.h"

#define MAX 20

int max(int, int);

int main(int argc, char *argv[]){
  int ip, gate, block = 0;
  int key;
  char succ_ip[20];
  char succ_gate[20];
  char s_succ_ip[20];
  char s_succ_gate[20];

  if(argc != 3) exit(1);
  /*else{
    sscanf(argv[1], "%d", &ip);
    sscanf(argv[2], "%d", &gate);
  }
  printf("ip: %d\n", ip);
  printf("gate: %d\n", gate);*/
  struct Server udp_server = init_udp_sv(argv[2]);
  struct Server tcp_server = init_tcp_sv(argv[2]);
  fd_set rfds;
  enum {idle, busy} state;
  int maxfd, counter, afd = 5;
  char buffer[10];

  state=idle;
  while(1){
    FD_ZERO(&rfds);
    FD_SET(udp_server.fd, &rfds);
    /*sprintf(buffer, "%d", udp_server.fd);
    printf("udp : %s\n", buffer);*/
    FD_SET(tcp_server.fd, &rfds);
    /*sprintf(buffer, "%d", tcp_server.fd);
    printf("tcp : %s\n", buffer);*/
    if(state==busy){
      FD_SET(afd, &rfds);
      maxfd = max(maxfd, afd) + 1;
    }
    FD_SET(0, &rfds);
    maxfd = max(udp_server.fd, tcp_server.fd) + 1;

    counter = select(maxfd, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);

    if(counter <= 0) /*error*/
      exit(1);

    if(FD_ISSET(udp_server.fd, &rfds)){
      udp_server = listen_udp_sv(udp_server);
    }

    if(FD_ISSET(tcp_server.fd, &rfds)){
      tcp_server = listen_tcp_sv(tcp_server);
    }

    /**************************
    READING INPUT FROM KEYBOARD
    **************************/
    if(FD_ISSET(0, &rfds)){
      fgets(buffer, MAX, stdin);
      char *token = strtok(buffer, " ");

      /*NEW: creating the first server*/
      if(strcmp(token, "new") == 0 && block == 0){
        token = strtok(NULL, " ");
        if(strcmp(token, "\n") == 0 || strcmp(token, " ") == 0){
          printf("-> Invalid command.\n");
          continue;
        }
        key = atoi(token);
        key = atoi(token);
        strcpy(succ_ip, argv[1]);
        strcpy(succ_gate, argv[2]);
        strcpy(s_succ_ip, argv[1]);
        strcpy(s_succ_gate, argv[2]);
        block = 1;
        printf("-> Server created.\n");
      }

      /*ENTRY: ... */
      else if(strcmp(token, "entry") == 0 && block == 0){
        token = strtok(NULL, " ");
        if(strcmp(token, "\n") == 0 || strcmp(token, " ") == 0){
          printf("-> Invalid command.\n");
          continue;
        }
        key = atoi(token);
        key = atoi(token);

        /* do stuff */

        block = 1;
        printf("-> Server entered.\n");
      }

      /*SENTRY: adding a server specifying it's successor*/
      else if(strcmp(token, "sentry") == 0 && block == 0){
        token = strtok(NULL, " ");
        if(strcmp(token, "\n") == 0 || strcmp(token, " ") == 0){
          printf("-> Invalid command.\n");
          continue;
        }
        key = atoi(token);
        key = atoi(token);
        /*value of the successors ip*/
        token = strtok(NULL, " ");
        if(strcmp(token, "\n") == 0 || strcmp(token, " ") == 0){
          printf("-> Invalid command.\n");
          continue;
        }
        printf("%s -this is succ_ip\n", token);
        strcpy(succ_ip, token);
        /*value of the successors gate*/
        token = strtok(NULL, " ");
        if(strcmp(token, "\n") == 0 || strcmp(token, " ") == 0){
          printf("-> Invalid command.\n");
          continue;
        }
        token[strlen(token)-1] = '\0'; /*cutting the \n*/
        strcpy(succ_gate, token);

        /*test for unique case when there are only 2 servers*/
        /*otherwise do the normal procedure*/

        block = 1;
        printf("-> Server sentered.\n");
      }

      /*LEAVE: ... */
      else if(strcmp(token, "leave\n") == 0 && block == 1){
          /* do stuff */
      }

      /* FALTA ADICIONAR O ESTADO DO SERVIDOR!!! */
      else if(strcmp(token, "show\n") == 0 && block == 1){
          printf("-> Key: %d\n-> IP: %s\n-> PORT: %s\n-> SuccIP: %s\n"
                    "-> SuccPORT: %s\n", key, argv[1], argv[2],
                      succ_ip, succ_gate);
      }

      /*FIND: ... */
      else if(strcmp(token, "find") == 0){
          /* do stuff */
      }
      /*EXIT: exits the application successfully*/
      else if(strcmp(token, "exit\n") == 0){
          printf("\nExiting the application...\n");
          exit(EXIT_SUCCESS);
      }
      /*Invalid command, ignores it*/
      else printf("-> Invalid command.\n");
    }
  }
  close_tcp_sv(tcp_server);
  close_udp_sv(udp_server);
}

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}
