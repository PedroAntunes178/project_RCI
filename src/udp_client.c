#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

struct Program_connection init_udp_cl(char* ip, char* gate){

  struct Program_connection client;

  client.fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (client.fd == -1) /**error*/ exit(1);

  memset (&client.hints, 0, sizeof client.hints);
  client.hints.ai_family = AF_INET;
  client.hints.ai_socktype = SOCK_DGRAM;

  client.errcode = getaddrinfo(ip, gate, &client.hints, &client.res);
  if (client.errcode != 0) /*error*/  exit(1);

  return client;
}

struct Program_connection request_udp_cl(struct Program_connection client, char* msg){

  client.n = sendto(client.fd, msg, sizeof(msg), 0, client.res->ai_addr, client.res->ai_addrlen);
  if (client.n==-1) /*error*/ exit(1);

  client.addrlen = sizeof(client.addr);
  client.n = recvfrom(client.fd, client.buffer, 128, 0, (struct sockaddr*) &client.addr, &client.addrlen);
  if(client.n == -1) /*error*/ exit(-1);

  write(1, "echo: ", 6);
  write(1, client.buffer, client.n);

  return client;
}
