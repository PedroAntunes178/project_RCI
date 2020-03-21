#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

#define MAX 100

struct Program_connection init_tcp_sv(char* gate){

  struct Program_connection server;

  server.fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server.fd == -1) /**error*/ exit(1);

  memset(&server.hints, 0, sizeof server.hints);
  server.hints.ai_family = AF_INET;
  server.hints.ai_socktype = SOCK_STREAM;
  server.hints.ai_flags = AI_PASSIVE;

  server.errcode = getaddrinfo(NULL, gate, &server.hints, &server.res);
  if (server.errcode != 0) /*error*/  exit(1);

  server.n = bind(server.fd, server.res->ai_addr, server.res->ai_addrlen);
  if(server.n == -1) /*error*/ exit(-1);

  if (listen(server.fd,5) == -1) /*error*/ exit(1);

  return server;
}

struct Program_connection init_tcp_cl(char* ip, char* gate){

  struct Program_connection client;

  client.fd = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
  if(client.fd == -1) /*error*/ exit(1);

  memset(&client.hints, 0, sizeof client.hints);
  client.hints.ai_family = AF_INET;
  client.hints.ai_socktype = SOCK_STREAM;

  client.errcode = getaddrinfo(ip, gate, &client.hints, &client.res);
  if(client.errcode != 0) /*error*/ exit(1);

  client.n = connect(client.fd, client.res->ai_addr, client.res->ai_addrlen);
  if(client.n == -1) /*error*/ exit(1);

  return(client);
}

int take_a_decision(struct Program_connection received, int used_fd, struct Program_data my_data){

  int key;
  int succ_key;
  char* succ_ip;
  succ_ip = malloc((MAX+1)*sizeof(char));
  char* succ_gate;
  succ_gate = malloc((MAX+1)*sizeof(char));
  char* token;
  token = (char*)malloc((MAX+1)*sizeof(char));
  char* msg;
  msg = (char*)malloc((MAX+1)*sizeof(char));

  sscanf(received.buffer, "%s", token);

  /*SUCCCONF: Um servidor informa outro que este se tornou o seu sucessor. */
  if(strcmp(token, "SUCCCONF") == 0 && block == 0){
    if(sscanf(buffer, "%*s%c", &eol) == 1 && eol == '\n'){
      sprintf(msg, "SUCC %d %s %s\n", my_data.succ_key, my_data.succ_ip, my_data.succ_gate);
      received.n = write(used_fd, msg, strlen(msg));
      if (received.n==-1) /*error*/ exit(1);
    }
    else{
      printf("-> The command \\SUCCCONF is of type \"SUCCCONF\\n\".\n");
      return -1;
    }
  }

  /*SUCC: Um servidor informa o seu predecessor que o seu sucessor é succ com endereço
  IP succ.IP e porto succ.port.*/
  else if(strcmp(token, "SUCC") == 0 && block == 0){
    if(sscanf(buffer, "%*s %d %s %s%c", &succ_key, succ_ip, succ_gate, &eol) == 4 && eol == '\n'){
      my_data.s_succ_ip = succ_ip;
      my_data.s_succ_gate = succ_gate;
      printf("Entrou depois completo\n");
    }
    else{
      printf("-> The command \\SUCC is of type \"SUCC succ succ.IP succ.port\\n\".\n");
    }
  }

  /*NEW: Esta mensagem é usada em dois contextos diferentes. (1) Um servidor entrante
  informa o seu futuro sucessor que pretende entrar no anel com chave i, endereço
  IP i.IP e porto i.port. (2) Um servidor informa o seu atual predecessor que o
  servidor de chave i, endereço IP i.IP e porto i.port pretende entrar no anel,
  para que o predecessor estabeleça o servidor entrante como seu sucessor.*/
  else if(strcmp(token, "NEW") == 0 && block == 0){
    if(sscanf(buffer, "%*s %d%c", &key, &eol) == 2 && eol == '\n'){
    }
    else{
    }
  }

  /*FND: Um servidor delega no seu sucessor a pesquisa da chave k, a qual foi iniciada pelo
  servidor i com endereço IP i.IP e porto i.port. (O caráter \n delimita todas
  as mensagens enviadas sobre TCP.)*/
  else if(strcmp(token, "FND") == 0 && block == 0){
    if(sscanf(buffer, "%*s %d%c", &key, &eol) == 2 && eol == '\n'){
    }
    else{
    }
  }

  /*KEY: Um servidor informa o servidor que iniciou a pesquisa da chave k que esta chave
  se encontra armazenada no seu sucessor succ com endereço IP succ.IP e porto
  succ.port. Esta mensagem é enviada sobre uma sessão TCP criada para o
  4efeito, do servidor que pretende enviar a mensagem para o servidor que iniciou a
  pesquisa.*/
   else if(strcmp(token, "KEY") == 0 && block == 0){
     if(sscanf(buffer, "%*s %d%c", &key, &eol) == 2 && eol == '\n'){
     }
     else{
     }
   }

  /*Invalid command, ignores it*/
  else{
    printf("-> Invalid message.\n");
  }
  return 0;//on sucess
}
