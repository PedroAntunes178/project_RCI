#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

struct Connection init_udp_sv(char* gate){

  struct Connection server;

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

struct Connection listen_udp_sv(struct Connection server){

  server.addrlen = sizeof(server.addr);

  server.n = recvfrom(server.fd, server.buffer, 128, 0, (struct sockaddr*) &server.addr, &server.addrlen);
  if (server.n==-1) /*error*/ exit(1);
  write(1, "received: ", 9);
  write(1, server.buffer, server.n);

  server.n = sendto(server.fd, server.buffer, server.n, 0, (struct sockaddr*) &server.addr, server.addrlen);
  if (server.n==-1) /*error*/ exit(1);

  return server;
}

void close_udp_sv(struct Connection server){
  freeaddrinfo(server.res);
  close(server.fd);
}
