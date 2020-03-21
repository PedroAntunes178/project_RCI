#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

struct Connection init_tcp_cl(char* ip, char* gate){

  struct Connection client;

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
