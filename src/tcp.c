#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "server.h"

#define MAXKEY 16

/*
** init_tcp_sv( char* )
** -> retorna e inicializa um servidor TCP.
*/
struct Program_connection init_tcp_sv(char* gate){

  struct Program_connection server;

  memset(server.buffer, 0, MAX);

  server.fd = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
  if (server.fd == -1) /**error*/ exit(1);

  memset(&server.hints, 0, sizeof server.hints);
  server.hints.ai_family = AF_INET; //IPv4
  server.hints.ai_socktype = SOCK_STREAM; //TCP socket
  server.hints.ai_flags = AI_PASSIVE;

  server.errcode = getaddrinfo(NULL, gate, &server.hints, &server.res);
  if (server.errcode != 0) /*error*/  exit(1);

  server.n = bind(server.fd, server.res->ai_addr, server.res->ai_addrlen);
  if(server.n == -1) /*error*/ exit(-1);

  if (listen(server.fd,5) == -1) /*error*/ exit(1);

  return server;
}


/*
** init_tcp_cl( char* , char* )
** -> retorna e inicializa um cliente TCP.
*/
struct Program_connection init_tcp_cl(char* ip, char* gate){

  struct Program_connection client;

  memset(client.buffer, 0, MAX);

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


/*
** init_tcp_cl( char* , char* )
** -> trata de duas mensagens protoculares ( NEW , KEY ) retornando o afd.
*/
int new_conection_to_me(int afd, int new_conection_fd, char* buffer, struct Program_data my_data, struct Program_connection udp_server){

  int n;
  char eol = 0;
  char* token = calloc(MAX, sizeof(char));
  char* msg = calloc(MAX, sizeof(char));

  int find_key = 0;
  int copy_key;
  char* copy_ip = calloc(MAX, sizeof(char));
  char* copy_gate = calloc(MAX, sizeof(char));

  //vamos ter de arranjar forma para confirmar que a msg que queremos ler está toda no buffer
  sscanf(buffer, "%s", token);
  fprintf(stderr, "New message received: %s", buffer);

  /*NEW: Esta mensagem é usada em dois contextos diferentes. (1)(este caso) Um servidor entrante
  informa o seu futuro sucessor que pretende entrar no anel com chave i, endereço
  IP i.IP e porto i.port. (2) Um servidor informa o seu atual predecessor que o
  servidor de chave i, endereço IP i.IP e porto i.port pretende entrar no anel,
  para que o predecessor estabeleça o servidor entrante como seu sucessor.*/
  if(strcmp(token, "NEW") == 0){
    if(sscanf(buffer, "%*s %d %s %s%c", &copy_key, copy_ip, copy_gate, &eol) == 4 && eol == '\n'){
      n = write(afd, buffer, strlen(buffer));
      if (n==-1) /*error*/ exit(1);
      if(my_data.key==my_data.succ_key){
        //significa que é o segundo a se juntar ao anel
        sprintf(msg, "SUCC %d %s %s\n", copy_key, copy_ip, copy_gate);
        n = write(new_conection_fd, msg, strlen(msg));
        if (n==-1) /*error*/ exit(1);
      }
      else{
        sprintf(msg, "SUCC %d %s %s\n", my_data.succ_key, my_data.succ_ip, my_data.succ_gate);
        n = write(new_conection_fd, msg, strlen(msg));
        if (n==-1) /*error*/ exit(1);
      }
      free(token);
      free(msg);
      free(copy_ip);
      free(copy_gate);
      return new_conection_fd;
    }
    else{
      fprintf(stderr, "ERROR -> The command \\NEW is of type \"NEW i i.IP i.port\\n\".\n");
    }
  }

  /*KEY: Um servidor informa o servidor que iniciou a pesquisa da chave k que esta chave
  se encontra armazenada no seu sucessor succ com endereço IP succ.IP e porto
  succ.port. Esta mensagem é enviada sobre uma sessão TCP criada para o
  efeito, do servidor que pretende enviar a mensagem para o servidor que iniciou a
  pesquisa.*/
  else if(strcmp(token, "KEY") == 0){
    if(sscanf(buffer, "%*s %d %d %s %s%c", &find_key, &copy_key, copy_ip, copy_gate, &eol) == 5 && eol == '\n'){
      fprintf(stdout, "->Key found: %d\n", find_key);
      fprintf(stdout, "->Found key owner key: %d\n", copy_key);
      fprintf(stdout, "->Found key owner ip: %s\n", copy_ip);
      fprintf(stdout, "->Found key owner gate: %s\n", copy_gate);
      close(new_conection_fd);
      if(my_data.asked_for_entry){
        memset(msg, 0, MAX);
        sprintf(msg, "EKEY %d %d %s %s\n", find_key, copy_key, copy_ip, copy_gate);
        n = sendto(udp_server.fd, msg, strlen(msg), 0, (struct sockaddr*) &udp_server.addr, udp_server.addrlen);
        if (n==-1) exit(1);
      }
    }
    else{
      fprintf(stderr, "ERROR -> The command \\KEY is of type \"KEY k succ succ.IP succ.port\\n\".\n");
    }
  }
  free(token);
  free(msg);
  free(copy_ip);
  free(copy_gate);
  return afd;
}


/*
** take_a_decision( struct Program_connection* , int , int , struct Program_data* )
** -> trata de várias mensagens protoculares ( SUCCCONF , SUCC , NEW , FND )
**    retornando 0 caso o comando seja válido.
*/
int take_a_decision(struct Program_connection* received, int response_fd, int pass_the_message_fd, struct Program_data* my_data){

  int key;
  char eol = 0;
  char token[MAX];
  memset(token, 0, MAX);
  char msg[MAX];
  memset(msg, 0, MAX);

  int og_key;             /* chave do: servidor que fez o pedido em FND ou do servidor que tem a chave em KEY*/
  int own_dist;           /* distância do próprio servidor à chave */
  int succ_dist;          /* distância do sucessor à chave */
  char og_ip[MIN];        /* ip do: servidor que fez o pedido em FND ou do servidor que tem a chave em KEY */
  memset(og_ip, 0, MIN);
  char og_gate[MIN];      /* porta do: servidor que fez o pedido em FND ou do servidor que tem a chave em KEY */
  memset(og_gate, 0, MIN);
  struct Program_connection tcp_sendkey;  /* cliente temporário para enviar info do FND */


  sscanf(received->buffer, "%s", token);

  /*SUCCCONF: Um servidor informa outro que este se tornou o seu sucessor. */
  if(strcmp(token, "SUCCCONF") == 0){
    if(sscanf(received->buffer, "%*s%c", &eol) == 1 && eol == '\n'){
      sprintf(msg, "SUCC %d %s %s\n", my_data->succ_key, my_data->succ_ip, my_data->succ_gate);
      received->n = write(response_fd, msg, strlen(msg));
      if (received->n==-1) /*error*/ exit(1);
    }
    else{
      fprintf(stderr, "-> The command \\SUCCCONF is of type \"SUCCCONF\\n\".\n");
      return -1;
    }
  }

  /*SUCC: Um servidor informa o seu predecessor que o seu sucessor é succ com endereço
  IP succ.IP e porto succ.port.*/
  else if(strcmp(token, "SUCC") == 0){
    if(sscanf(received->buffer, "%*s %d %s %s%c", &my_data->s_succ_key, my_data->s_succ_ip, my_data->s_succ_gate, &eol) == 4 && eol == '\n'){
      fprintf(stderr, "Information about the sucessor of my sucessor arived.\n");
    }
    else{
      fprintf(stderr, "-> The command \\SUCC is of type \"SUCC succ succ.IP succ.port\\n\".\n");
      return -1;
    }
  }

  /*NEW: Esta mensagem é usada em dois contextos diferentes. (1) Um servidor entrante
  informa o seu futuro sucessor que pretende entrar no anel com chave i, endereço
  IP i.IP e porto i.port. (2)(este caso) Um servidor informa o seu atual predecessor que o
  servidor de chave i, endereço IP i.IP e porto i.port pretende entrar no anel,
  para que o predecessor estabeleça o servidor entrante como seu sucessor.*/
  else if(strcmp(token, "NEW") == 0){
    if(sscanf(received->buffer, "%*s %d %s %s%c", &my_data->succ_key, my_data->succ_ip, my_data->succ_gate, &eol) == 4 && eol == '\n'){
      freeaddrinfo(received->res);
      close(response_fd);
      *received = init_tcp_cl(my_data->succ_ip, my_data->succ_gate);

      sprintf(msg, "SUCCCONF\n");
      received->n = write(response_fd, msg, strlen(msg));
      if (received->n==-1) /*error*/ exit(1);

      sprintf(msg, "SUCC %d %s %s\n", my_data->succ_key, my_data->succ_ip, my_data->succ_gate);
      received->n = write(pass_the_message_fd, msg, strlen(msg));
      if (received->n==-1) /*error*/ exit(1);
    }
    else{
      fprintf(stderr, "-> The command \\NEW is of type \"NEW i i.IP i.port\\n\".\n");
      return -1;
    }
  }

  /*FND: Um servidor delega no seu sucessor a pesquisa da chave k, a qual foi iniciada pelo
  servidor i com endereço IP i.IP e porto i.port. (O caráter \n delimita todas
  as mensagens enviadas sobre TCP.)*/
  else if(strcmp(token, "FND") == 0){
    if(sscanf(received->buffer, "%*s %d %d %s %s%c", &key, &og_key, og_ip, og_gate, &eol) == 5 && eol == '\n'){
      /* calculate the distance from the key to the own server */
      if((my_data->key - key) < 0)
        own_dist = my_data->key - key + MAXKEY;
      else own_dist = my_data->key - key;
      /*calculate the distance from the key to the successor server */
      if((my_data->succ_key - key) < 0)
        succ_dist = my_data->succ_key - key + MAXKEY;
      else succ_dist = my_data->succ_key - key;
      /* compares the two distances and acts accordingly */
      if(own_dist < succ_dist){
        sprintf(msg, "FND %d %d %s %s\n", key, og_key, og_ip, og_gate);
        received->n = write(pass_the_message_fd, msg, MAX);
        if(received->n == -1) /*error*/ exit(1);
      }
      else{
        tcp_sendkey = init_tcp_cl(og_ip, og_gate);
        sprintf(msg, "KEY %d %d %s %s\n", key, my_data->succ_key, my_data->succ_ip, my_data->succ_gate);
        tcp_sendkey.n = write(tcp_sendkey.fd, msg, MAX);
        if(tcp_sendkey.n == 1) /*error*/ exit(1);
        fprintf(stdout, "-> Key found in my successor.\n");
        freeaddrinfo(tcp_sendkey.res);
        close(tcp_sendkey.fd);
      }
    }
    else{
      fprintf(stderr, "-> The command \\FND is of type \"FND k i i.IP i.port\\n\".\n");
      return -1;
    }
  }
  else{
    fprintf(stderr, "Received invalid TCP message.\n");
    return -1;
  }
  memset(received->buffer, 0 , MAX);
  return 0;//on sucess
}
