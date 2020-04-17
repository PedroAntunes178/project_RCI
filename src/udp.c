#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

struct Program_connection init_udp_sv(char* gate){

  struct Program_connection server;

  memset(server.buffer, 0, MAX);

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


struct Program_connection init_udp_cl(char* ip, char* gate){

  struct Program_connection client;

  memset(client.buffer, 0, MAX);

  client.fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (client.fd == -1) /**error*/ exit(1);

  memset (&client.hints, 0, sizeof client.hints);
  client.hints.ai_family = AF_INET;
  client.hints.ai_socktype = SOCK_DGRAM;

  client.errcode = getaddrinfo(ip, gate, &client.hints, &client.res);
  if (client.errcode != 0) /*error*/  exit(1);

  return client;
}
