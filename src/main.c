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

#define MAX 100

int max(int, int);

int main(int argc, char *argv[]){

  if(argc != 3) exit(1);

  int key;
  int succ_key;
  char* succ_ip;
  succ_ip = malloc((MAX+1)*sizeof(char));
  char* succ_gate;
  succ_gate = malloc((MAX+1)*sizeof(char));
  char* s_succ_ip;
  s_succ_ip = malloc((MAX+1)*sizeof(char));
  char* s_succ_gate;
  s_succ_gate = malloc((MAX+1)*sizeof(char));

  struct Connection udp_server = init_udp_sv(argv[2]);
  struct Connection tcp_server = init_tcp_sv(argv[2]);
  struct Connection udp_client;
  struct Connection tcp_client;
  tcp_client.fd = -1;
  fd_set rfds;
  int state_cl=0;
  int state_sv=0;
  int maxfd, counter, newfd, afd = -1;

  char* buffer;
  buffer = (char*)malloc((MAX+1)*sizeof(char));
  char* token;
  token = (char*)malloc((MAX+1)*sizeof(char));
  char* msg;
  msg = (char*)malloc((MAX+1)*sizeof(char));
  char eol = 0;
  int block = 0;
  int exit_flag = 0;

  while(!(exit_flag)){
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(udp_server.fd, &rfds);
    FD_SET(tcp_server.fd, &rfds);
    maxfd = max(udp_server.fd, tcp_server.fd) + 1;
    if(state_cl){
      //printf("fd_set cl\n");
      FD_SET(tcp_client.fd, &rfds);
      maxfd = max(maxfd, tcp_client.fd) + 1;
    }
    if(state_sv){
      //printf("fd_set sv\n");
      FD_SET(afd, &rfds);
      maxfd = max(maxfd, afd) + 1;
    }

    counter = select(maxfd, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);

    if(counter <= 0) /*error*/
      exit(1);

    if(FD_ISSET(udp_server.fd, &rfds)){
      udp_server = listen_udp_sv(udp_server);
    }

    /* WAITING FOR CONNECTING AS TCP SERVER*/
    if(FD_ISSET(tcp_server.fd, &rfds)){
      tcp_server.addrlen = sizeof(tcp_server.addr);
      if((newfd = accept(tcp_server.fd, (struct sockaddr*) &tcp_server.addr,
            &tcp_server.addrlen)) == -1) /*error*/ exit(1);
      printf("CONNECTION DONE\n");
      if(!(state_sv)){
        afd = newfd;
        printf("newfd : %d\n", newfd);
        state_sv = 1;
      }
      else{
        //send message sying "I'm busy_sv"
        close(newfd);
      }
    }

    /* WAITING TO READ AS TCP SERVER*/
    if(FD_ISSET(afd, &rfds)){
      //printf("Entrou!!\n");
      if((tcp_server.n = read(afd, tcp_server.buffer, 128)) != 0){
        if(tcp_server.n == -1) /*error*/ exit(1);//é aqui que está a retornar um erro
        write(1, "received: ", 10);
        write(1, tcp_server.buffer, tcp_server.n);
        tcp_server.n = write(afd, tcp_server.buffer, tcp_server.n);
        if (tcp_server.n==-1) /*error*/ exit(1);
      }
      else{
        printf("Closed Server connection.\n");
        close(afd);
        state_sv = 0;
      }
    }

    /* WAITING TO READ AS TCP CLIENT */
    if(FD_ISSET(tcp_client.fd, &rfds)){
      if((tcp_client.n = read(tcp_client.fd, tcp_client.buffer, 128)) != 0){
        if(tcp_client.n == -1) /*error*/ exit(1);//é aqui que está a retornar um erro
        write(1, "echo: ", 6);
        write(1, tcp_client.buffer, tcp_client.n);
      }
      else{
        printf("Closed Client connection.\n");
        freeaddrinfo(tcp_client.res);
        close(tcp_client.fd);
        state_cl = 0;
      }
    }

    /**************************
    READING INPUT FROM KEYBOARD
    **************************/
    if(FD_ISSET(0, &rfds)){
      fgets(buffer, MAX, stdin);
      sscanf(buffer, "%s", token);

      /*NEW: creating the first server*/
      if(strcmp(token, "new") == 0 && block == 0){
        if(sscanf(buffer, "%*s %d%c", &key, &eol) == 2 && eol == '\n'){
          strcpy(succ_ip, argv[1]);
          strcpy(succ_gate, argv[2]);
          strcpy(s_succ_ip, argv[1]);
          strcpy(s_succ_gate, argv[2]);
          block = 1;
          printf("Chave : %d\n", key);
          printf("-> Ring created.\n");
        }
        else{
          printf("-> The command \\new is of type \"new i\". Where i is a key.\n");
          memset(buffer, 0, MAX);
          memset(token, 0, MAX);
        }
      }

      /*ENTRY: ... */
      else if(strcmp(token, "entry") == 0 && block == 0){

        /* do stuff */

        block = 1;
        printf("-> Server entered.\n");
      }

      /*SENTRY: adding a server specifying it's successor */
      else if(strcmp(token, "sentry") == 0 && block == 0){
        if(sscanf(buffer, "%*s %d %d %s %s%c", &key, &succ_key, succ_ip, succ_gate, &eol) == 5 && eol == '\n'){
          strcpy(msg, "Olá outra porta\n");
          tcp_client = init_tcp_cl(succ_ip, succ_gate);
          state_cl = 1;

          tcp_client.n = write(tcp_client.fd, msg, MAX);
          if(tcp_client.n == -1) /*error*/ exit(1);
          write(1, "client message: \n", 17);
          write(1, msg, tcp_client.n);
          /* do stuff */

          printf("Chave : %d\n", key);
          printf("Next server ip: %s\n", succ_ip);
          printf("Next server ip: %s\n", succ_gate);
          block = 1;
          printf("-> Server sentered.\n");
        }
        else{
          printf("-> The command \\sentry is of type \"sentry i succ.ip succ.gate\". Where i is a key.\n");
          memset(buffer, 0, MAX);
          memset(token, 0, MAX);
        }
      }

      /*LEAVE: ... */
      else if(strcmp(buffer, "leave\n") == 0 && block == 1){
          /* do stuff */
          freeaddrinfo(tcp_client.res);
          close(tcp_client.fd);
          state_cl = 0;
          block = 0;
          printf("-> Left the ring.\n");
      }

      /* FALTA ADICIONAR O ESTADO DO SERVIDOR!!! */
      else if(strcmp(buffer, "show\n") == 0 && block == 1){
          printf("-> Key: %d\n-> IP: %s\n-> PORT: %s\n-> SuccIP: %s\n"
                    "-> SuccPORT: %s\n", key, argv[1], argv[2],
                      succ_ip, succ_gate);
      }

      /*FIND: ... */
      else if(strcmp(token, "find") == 0){
          /* do stuff */
      }
      /*EXIT: exits the application successfully*/
      else if(strcmp(buffer, "exit\n") == 0){
          printf("\nExiting the application...\n");
          exit_flag = 1;
      }
      /*Invalid command, ignores it*/
      else{
        printf("-> Invalid command.\n");
      }
    }
  }

  freeaddrinfo(tcp_server.res);
  close(tcp_server.fd);
  freeaddrinfo(udp_server.res);
  close(udp_server.fd);
  free(succ_ip);
  free(succ_gate);
  free(s_succ_ip);
  free(s_succ_gate);
  free(buffer);
  free(token);
  exit(EXIT_SUCCESS);
}

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}
