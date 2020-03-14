#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

struct Server init_tcp_sv(char* gate){

  struct Server server;

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

struct Server listen_tcp_sv(struct Server server){

  server.addrlen = sizeof(server.addr);
  if((server.newfd = accept(server.fd, (struct sockaddr*) &server.addr,
        &server.addrlen)) == -1) /*error*/ exit(1);

  server.n = read(server.newfd, server.buffer, 128);
  if (server.n==-1) /*error*/ exit(1);
  write(1, "received: ", 10);
  write(1, server.buffer, server.n);

  server.n = write(server.newfd, server.buffer, server.n);
  if (server.n==-1) /*error*/ exit(1);

  close(server.newfd);

  return server;
}

void close_tcp_sv(struct Server server){
  freeaddrinfo(server.res);
  close(server.fd);
}
