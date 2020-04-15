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

struct Program_connection init_udp_sv(char* gate){

  struct Program_connection server;

  server.fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server.fd == -1) /**error*/ exit(1);

  memset(&server.hints, 0, sizeof server.hints);
  server.hints.ai_family = AF_INET;
  server.hints.ai_socktype = SOCK_DGRAM;
  server.hints.ai_flags = AI_PASSIVE;

  server.errcode = getaddrinfo(NULL, gate, &server.hints, &server.res);
  if (server.errcode != 0) /*error*/  exit(1);

  server.n = bind(server.fd, server.res->ai_addr, server.res->ai_addrlen);
  if(server.n == -1) /*error*/ exit(-1);

  return server;
}

/*********************************************************************************************/
int take_a_decision_udp(struct Program_connection received, int response_fd, struct Program_data* my_data){

	int key = 0;
	int succ_key = 0;
	char* succ_ip;
	succ_ip = malloc((101)*sizeof(char));
	char* succ_gate;
	succ_gate = malloc((101)*sizeof(char));


	char eol = 0;
	char* token;
  token = (char*)malloc((MAX+1)*sizeof(char));
  char* msg;
  msg = (char*)malloc((MAX+1)*sizeof(char));

	write(1, "received in udp: ", 17);
	write(1, received.buffer, strlen(received.buffer));

	sscanf(received.buffer, "%s", token);

	/*EFND: Um servidor solicita outro a sua posição no anel. */
  if(strcmp(token, "EFND") == 0){
    if(sscanf(received.buffer, "%*s %d%c", &key, &eol) == 2 && eol == '\n'){
      printf("EXECUTAR O FIND!\n");
    }
	}
	/*EKEY: O servidor recebe uma resposta com a sua posição no anel. */
  else if(strcmp(token, "EKEY") == 0){
    if(sscanf(received.buffer, "%*s %d %d %s %s%c", &key, &succ_key, succ_ip, succ_gate, &eol) == 5 && eol == '\n'){
      printf("EXECUTAR O SENTRY!\n");
    }
	}
}
/*********************************************************************************************/

struct Program_connection listen_udp_sv(struct Program_connection server){

  server.addrlen = sizeof(server.addr);

  server.n = recvfrom(server.fd, server.buffer, 128, 0, (struct sockaddr*) &server.addr, &server.addrlen);
  if (server.n==-1) /*error*/ exit(1);
  write(1, "received: ", 9);
  write(1, server.buffer, server.n);

  server.n = sendto(server.fd, server.buffer, server.n, 0, (struct sockaddr*) &server.addr, server.addrlen);
  if (server.n==-1) /*error*/ exit(1);

  return server;
}
