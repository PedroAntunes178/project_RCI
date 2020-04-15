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
#define MAXKEY 16


int main(int argc, char *argv[]){

  if(argc != 3){
    exit(1);
    fprintf(stderr, "Por favor chamar o programa tipo ./dkt <ip> <porto>\n");
  }

  struct Program_data my_data;
  my_data = init_program_data();
  strcpy(my_data.ip, argv[1]);
  strcpy(my_data.gate, argv[2]);

  struct Program_connection udp_server = init_udp_sv(argv[2]);
  struct Program_connection tcp_server = init_tcp_sv(argv[2]);
  struct Program_connection udp_client;
  struct Program_connection tcp_client;
  tcp_client.fd = -1;
  fd_set rfds;
  int maxfd, counter, newfd, afd = -1, new_conection_fd = -1;

  int find_key;

  char* buffer;
  buffer = (char*)malloc((MAX+1)*sizeof(char));
  char* token;
  token = (char*)malloc((MAX+1)*sizeof(char));
  char* msg;
  msg = (char*)malloc((MAX+1)*sizeof(char));
  char* received_message_buffer;
  received_message_buffer = (char*)malloc((MAX+1)*sizeof(char));
  char eol = 0;
  int inside_a_ring = 0;
  int exit_flag = 0;

  while(1){
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(udp_server.fd, &rfds);
    FD_SET(tcp_server.fd, &rfds);
    maxfd = max(udp_server.fd, tcp_server.fd) + 1;
    /*
    A sequência de ifs que se seguem servem para não inicializar os file descriptors sem eles serem necessários.
    */
    if(my_data.state_new_conection){
      //printf("fd_set new_sv\n");
      FD_SET(new_conection_fd, &rfds);
      maxfd = max(maxfd, new_conection_fd) + 1;
    }
    if(my_data.state_sv){
      //printf("fd_set sv\n");
      FD_SET(afd, &rfds);
      maxfd = max(maxfd, afd) + 1;
    }
    if(my_data.state_cl){
      //printf("fd_set cl\n");
      FD_SET(tcp_client.fd, &rfds);
      maxfd = max(maxfd, tcp_client.fd) + 1;
    }

    counter = select(maxfd, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);

    if(counter <= 0) /*error*/
      exit(1);

    if(FD_ISSET(udp_server.fd, &rfds)){
      //udp_server = listen_udp_sv(udp_server);
    }

    /* WAITING FOR CONNECTING AS TCP SERVER*/
    if(FD_ISSET(tcp_server.fd, &rfds)){
      tcp_server.addrlen = sizeof(tcp_server.addr);
      if((newfd = accept(tcp_server.fd, (struct sockaddr*) &tcp_server.addr,
            &tcp_server.addrlen)) == -1) /*error*/ exit(1);
      if(!(my_data.state_sv)){
        afd = newfd;
        my_data.state_sv = 1;
        fprintf(stderr, "First connection done successfully.\n");
      }
      else if(!(my_data.state_new_conection)){
        new_conection_fd = newfd;
        my_data.state_new_conection = 1;
        fprintf(stderr, "Received new conection data.\n");
      }
      else{
      fprintf(stderr, "To many connecction to be processed.\n");
      close(newfd);
      }
    }

    /* WAITING TO READ AS TCP SERVER*/
    if(FD_ISSET(new_conection_fd, &rfds)){
      //printf("Entrou!!\n");
      /*Se  new_connection retornar 0 seguenifica que está td bem.*/
      new_conection_to_me(&afd, new_conection_fd, my_data);
      new_conection_fd = -1;
      my_data.state_new_conection = 0;
      fprintf(stderr, "New connection processed successfully.\n");
    }

    /* WAITING TO READ AS TCP SERVER*/
    if(FD_ISSET(afd, &rfds)){
      //printf("Entrou!!\n");
      if((tcp_server.n = read(afd, tcp_server.buffer, 128)) != 0){
        if(tcp_server.n == -1) /*error*/ exit(1);
        fprintf(stdout, "Received: %s\n", tcp_server.buffer);
        take_a_decision(tcp_server, afd, tcp_client.fd, &my_data);
      }
      else{
        fprintf(stderr, "Connection lost with predecessor.\n");
        close(afd);
        my_data.state_sv = 0;
      }
    }


    /* WAITING TO READ AS TCP CLIENT */
    if(FD_ISSET(tcp_client.fd, &rfds)){
      if((tcp_client.n = read(tcp_client.fd, tcp_client.buffer, 128)) != 0){
        if(tcp_client.n == -1) /*error*/ exit(1);
        fprintf(stdout, "echo: %s\n", tcp_client.buffer);
        take_a_decision(tcp_client, tcp_client.fd, afd, &my_data);
      }
      else{
        fprintf(stdout, "Connection lost with sucessor.\nEstablishing connection to new sucessor...\n");
        freeaddrinfo(tcp_client.res);
        close(tcp_client.fd);
        my_data.succ_ip = my_data.s_succ_ip;
        my_data.succ_gate = my_data.s_succ_gate;
        my_data.succ_key = my_data.s_succ_key;
        tcp_client = init_tcp_cl(my_data.succ_ip, my_data.succ_gate);
        sprintf(msg, "SUCCCONF\n");
        tcp_client.n = write(tcp_client.fd, msg, MAX);
        if(tcp_client.n == -1) /*error*/ exit(1);
        sprintf(msg, "SUCC %d %s %s\n", my_data.succ_key, my_data.succ_ip, my_data.succ_gate);
        tcp_server.n = write(afd, msg, MAX);
        if(tcp_server.n == -1) /*error*/ exit(1);
      }
    }


    /**************************
    READING INPUT FROM KEYBOARD
    **************************/
    if(FD_ISSET(0, &rfds)){
      fgets(buffer, MAX, stdin);
      sscanf(buffer, "%s", token);

      /*NEW: creating the first server*/
      if(strcmp(token, "new") == 0 && inside_a_ring == 0){
        if(sscanf(buffer, "%*s %d%c", &my_data.key, &eol) == 2 && eol == '\n'){
          my_data.succ_key = my_data.key;
          my_data.s_succ_key = my_data.key;
          strcpy(my_data.succ_ip, my_data.ip);
          strcpy(my_data.succ_gate, my_data.gate);
          strcpy(my_data.s_succ_ip, my_data.ip);
          strcpy(my_data.s_succ_gate, my_data.gate);
          fprintf(stdout, "merda\n");
          tcp_client = init_tcp_cl(my_data.succ_ip, my_data.succ_gate);
          my_data.state_cl = 1;
          inside_a_ring = 1;
          fprintf(stdout, "Created a new ring.\n");
        }
        else{
          fprintf(stderr, "-> The command \\new is of type \"new i\". Where i is a key.\n");
          memset(buffer, 0, MAX);
          memset(token, 0, MAX);
        }
      }

      /*ENTRY: ... */
      else if(strcmp(token, "entry") == 0 && inside_a_ring == 0){

        /* do stuff */

        inside_a_ring = !(sentry(&my_data, tcp_client, msg));
        fprintf(stdout, "-> Server entered.\n");
      }

      /*SENTRY: adding a server specifying it's successor */
      else if(strcmp(token, "sentry") == 0 && !(inside_a_ring)){
        if(sscanf(buffer, "%*s %d %d %s %s%c", &my_data.key, &my_data.succ_key, my_data.succ_ip, my_data.succ_gate, &eol) == 5 && eol == '\n'){
           if(sentry(&my_data, tcp_client, msg) == 0 ){
             inside_a_ring = 1;
           }
           else fprintf(stdout, "I can't have the same key as my sucessor.\n");
        }
        else{
          fprintf(stderr, "ERROR -> The command \\sentry is of type \"sentry i succ.ip succ.gate\". Where i is a key.\n");
          memset(buffer, 0, MAX);
          memset(token, 0, MAX);
        }
      }

      /*LEAVE: ... */
      else if(strcmp(buffer, "leave\n") == 0 && inside_a_ring){
        leave(tcp_client, afd, &my_data);
        inside_a_ring = 0;
      }

      /* FALTA ADICIONAR O ESTADO DO SERVIDOR!!! */
      else if(strcmp(buffer, "show\n") == 0){
        if(inside_a_ring)
          fprintf(stderr, "ERROR -> Key: %d\n-> IP: %s\n-> PORT: %s\n-> SuccKey: %d\n-> SuccIP: %s\n-> SuccPORT: %s\n-> S_SuccKey: %d\n-> S_SuccIP: %s\n-> S_SuccPORT: %s\n", my_data.key, my_data.ip, my_data.gate, my_data.succ_key, my_data.succ_ip, my_data.succ_gate, my_data.s_succ_key, my_data.s_succ_ip, my_data.s_succ_gate);
        else fprintf(stderr, "Not inside a ring, so I don't have a successor :( )\n");
      }

      /*FIND: ... */
      else if(strcmp(token, "find") == 0){
          if(sscanf(buffer, "%*s %d%c", &find_key, &eol) == 2 && eol == '\n'){
            sprintf(msg, "FND %d %d %s %s\n", find_key, my_data.key, my_data.ip, my_data.gate);
            tcp_client.n = write(tcp_client.fd, msg, MAX);
            if(tcp_client.n == -1) /*error*/ exit(1);
          }
          else{
            printf("-> The command \\find is of type \"find i\". Where i is a key.\n");
            memset(buffer, 0, MAX);
            memset(token, 0, MAX);
          }
      }
      /*EXIT: exits the application successfully*/
      else if(strcmp(buffer, "exit\n") == 0){
        leave(tcp_client, afd, &my_data);
        free_program_data(my_data);
        free(buffer);
        free(token);
        fprintf(stderr, "\nExiting the application...\n");
        exit(EXIT_SUCCESS);
      }
      /*Invalid command, ignores it*/
      else printf("-> Invalid command.\n");
    }
  }
}

int max(int x, int y){
  if (x > y)
    return x;
  else
    return y;
}

struct Program_data init_program_data(){
  struct Program_data init_data;
  init_data.ip = malloc((MAX+1)*sizeof(char));
  init_data.gate = malloc((MAX+1)*sizeof(char));
  init_data.succ_ip = malloc((MAX+1)*sizeof(char));
  init_data.succ_gate = malloc((MAX+1)*sizeof(char));
  init_data.s_succ_ip = malloc((MAX+1)*sizeof(char));
  init_data.s_succ_gate = malloc((MAX+1)*sizeof(char));
  init_data.state_cl=0;
  init_data.state_sv=0;
  init_data.state_new_conection=0;
  return init_data;
}

int free_program_data(struct Program_data free_data){
  free(free_data.ip);
  free(free_data.gate);
  free(free_data.succ_ip);
  free(free_data.succ_gate);
  free(free_data.s_succ_ip);
  free(free_data.s_succ_gate);
  return 0;
}

int leave(struct Program_connection tcp_client, int afd, struct Program_data* my_data){
  freeaddrinfo(tcp_client.res);
  close(tcp_client.fd);
  close(afd);
  my_data->state_cl = 0;
  my_data->state_sv = 0;
  my_data->state_new_conection = 0;
  fprintf(stdout, "Leaving the ring...\n");
  return 0;
}

int sentry(struct Program_data* my_data, struct Program_connection tcp_client, char* msg){

  if(my_data->key == my_data->succ_key) return 1;

  tcp_client = init_tcp_cl(my_data->succ_ip, my_data->succ_gate);
  my_data->state_cl = 1;

  sprintf(msg, "NEW %d %s %s\n", my_data->key, my_data->ip, my_data->gate);
  tcp_client.n = write(tcp_client.fd, msg, MAX);
  if(tcp_client.n == -1) /*error*/ exit(1);

  fprintf(stdout, "Key : %d\n", my_data->key);
  fprintf(stdout, "Next server key: %d\n", my_data->succ_key);
  fprintf(stdout, "Next server ip: %s\n", my_data->succ_ip);
  fprintf(stdout, "Next server gate: %s\n", my_data->succ_gate);
  fprintf(stderr, "-> Server sentered.\n");
  return 0;
 }
