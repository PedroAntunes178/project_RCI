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

int max(int, int);

int main(int argc, char *argv[]){
  int ip = 0;
  int gate = 0;
  if(argc != 3) exit(1);
  /*else{
    sscanf(argv[1], "%d", &ip);
    sscanf(argv[2], "%d", &gate);
  }
  printf("ip: %d\n", ip);
  printf("gate: %d\n", gate);*/
  struct Server udp_server = init_udp_sv(argv[2]);
  struct Server tcp_server = init_tcp_sv(argv[2]);
  fd_set rfds;
  enum {idle, busy} state;
  int maxfd, counter, afd = 5;
  char s[20];

  state=idle;
  while(1){
    FD_ZERO(&rfds);
    FD_SET(udp_server.fd, &rfds);
    /*sprintf(teste, "%d", udp_server.fd);
    printf("udp : %s\n", teste);*/
    FD_SET(tcp_server.fd, &rfds);
    /*sprintf(teste, "%d", tcp_server.fd);
    printf("tcp : %s\n", teste);*/
    if(state==busy){
      FD_SET(afd, &rfds);
      maxfd = max(maxfd, afd) + 1;
    }
    FD_SET(0, &rfds);
    maxfd = max(udp_server.fd, tcp_server.fd) + 1;

    counter = select(maxfd, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);

    if(counter <= 0) /*error*/
      exit(1);

    if(FD_ISSET(udp_server.fd, &rfds)){
      udp_server = listen_udp_sv(udp_server);
    }

    if(FD_ISSET(tcp_server.fd, &rfds)){
      tcp_server = listen_tcp_sv(tcp_server);
    }

    if(FD_ISSET(0, &rfds)){
      if(fscanf(stdin, "%s", s)>=0 && strcmp(s, "new")==0){
        printf("lol %s\n", s);
        if(fscanf(stdin, "%d", &tcp_server.key)==0){
          printf("litos %d\n", tcp_server.key);
        }
        else{
          printf("Nao foi dada uma Chave!\n");
        }
      }
    }
  }
  close_tcp_sv(tcp_server);
  close_udp_sv(udp_server);
}

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}
