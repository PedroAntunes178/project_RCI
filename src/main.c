#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

#define MAXKEY 16


int main(int argc, char *argv[]){

  if(argc != 3){
    fprintf(stderr, "Por favor chamar o programa tipo ./dkt <ip> <porto>\n");
    exit(1);
  }

  struct Program_data my_data;
  my_data = init_program_data();
  strcpy(my_data.ip, argv[1]);
  strcpy(my_data.gate, argv[2]);

  struct Program_connection udp_server = init_udp_sv(argv[2]);
  struct Program_connection tcp_server = init_tcp_sv(argv[2]);
  struct Program_connection tcp_client;
	struct Program_connection udp_client;
  tcp_client.fd = -1;
	udp_client.fd = -1;
  fd_set rfds;
  int maxfd, counter, newfd, afd = -1, new_conection_fd = -1;
  char* new_conection_buffer = calloc(MAX, sizeof(char));


  int find_key;

  int entry_sv_key = 0;			/*chave do servidor ao qual se solicita a entrada no anel*/
  char entry_sv_ip[MIN];		/*ip do servidor ao qual se solicita a entrada no anel*/
  char entry_sv_gate[MIN];	/*porto do servidor ao qual se solicita a entrada no anel*/
  int state_udp_cl = 0;
  int key_to_find = 0;

  char buffer[MAX];
  char token[MAX];
  char msg[MAX];
  char eol = 0;
  int inside_a_ring = 0;


  struct timeval _timeval;
  _timeval.tv_usec = 500;
  _timeval.tv_sec = 5;

  while(1){
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(udp_server.fd, &rfds);
    FD_SET(tcp_server.fd, &rfds);
    maxfd = max(udp_server.fd, tcp_server.fd) + 1;
    /*
    A sequência de ifs que se seguem servem para não inicializar os file descriptors sem eles serem necessários.
    */
    if(state_udp_cl){
      //fprintf(stderr, "fd_set udp_cl: %d\n", udp_client.fd);
      FD_SET(udp_client.fd, &rfds);
      maxfd = max(maxfd, udp_client.fd) + 1;
    }
    if(my_data.state_new_conection){
      //fprintf(stderr, "fd_set new_conection: %d\n", new_conection_fd);
      FD_SET(new_conection_fd, &rfds);
      maxfd = max(maxfd, new_conection_fd) + 1;
    }
    if(my_data.state_sv){
      //fprintf(stderr, "fd_set sv: %d\n", afd);
      FD_SET(afd, &rfds);
      maxfd = max(maxfd, afd) + 1;
    }
    if(my_data.state_cl){
      //fprintf(stderr, "fd_set cl: %d\n", tcp_client.fd);
      FD_SET(tcp_client.fd, &rfds);
      maxfd = max(maxfd, tcp_client.fd) + 1;
    }


    counter = select(maxfd, &rfds, (fd_set*)NULL, (fd_set*)NULL, &_timeval);
    if(counter == 0){
      if(state_udp_cl && !inside_a_ring){
        memset(msg, 0, MAX);
        sprintf(msg,"EFND %d\n", my_data.key);
        udp_client.n = sendto(udp_client.fd, msg, strlen(msg), 0, udp_client.res->ai_addr, udp_client.res->ai_addrlen);
        if(udp_client.n == -1) /*error*/ exit(1);
        fprintf(stderr, "-> Sent message as udp client: %s", msg);
      }
      _timeval.tv_sec = _timeval.tv_sec + 5;
    }
    else if(counter < 0) /*error*/
      exit(1);

    /* WAITING TO READ AS UDP SERVER */
    if(FD_ISSET(udp_server.fd, &rfds)){
      memset(udp_server.buffer, 0, MAX);
      udp_server.addrlen = sizeof(udp_server.addr);
      if((udp_server.n = recvfrom(udp_server.fd, udp_server.buffer, 128, 0, (struct sockaddr*) &udp_server.addr, &udp_server.addrlen)) != 0){
        if (udp_server.n==-1) /*error*/ exit(1);
        sscanf(udp_server.buffer, "%s", token);
        fprintf(stderr, "Received data in udp_server: %s", udp_server.buffer);
        /*EFND: Um servidor solicita a si qual a posição que lhe corresponde de acordo com a chave*/
        if(strcmp(token, "EFND") == 0){
          if(sscanf(udp_server.buffer, "%*s %d%c", &key_to_find, &eol) == 2 && eol == '\n'){
            my_data.asked_for_entry = 1;
            fprintf(stdout, "Executing find in the ring!\nAnd the sending EKEY to the origin.\n");
            memset(msg, 0, MAX);
            sprintf(msg, "FND %d %d %s %s\n", key_to_find, my_data.key, my_data.ip, my_data.gate);
            tcp_client.n = write(tcp_client.fd, msg, MAX);
            if(tcp_client.n == -1)  exit(1);

          }
        }
      }
    }


    /* WAITING TO READ AS UDP CLIENT */
    if(FD_ISSET(udp_client.fd, &rfds)){
      memset(udp_client.buffer, 0, MAX);
      udp_client.addrlen = sizeof(udp_client.addr);
      if((udp_client.n = recvfrom(udp_client.fd, udp_client.buffer, 128, 0, (struct sockaddr*) &udp_client.addr, &udp_client.addrlen)) != 0){
        if(udp_client.n == -1) /*error*/ exit(1);
        fprintf(stdout, "Received message as client: %s", udp_client.buffer);
        sscanf(udp_client.buffer, "%s", token);
        /*EKEY: O servidor recebe uma resposta com a sua posição no anel. */
        if(strcmp(token, "EKEY") == 0){
          if(sscanf(udp_client.buffer, "%*s %d %d %s %s%c", &my_data.key, &my_data.succ_key, my_data.succ_ip, my_data.succ_gate, &eol) == 5 && eol == '\n'){
            fprintf(stderr, "Executing sentry after receiving my successor!\n");
            if(sentry(&my_data, &tcp_client, msg) == 0 ){
              inside_a_ring = 1;
              state_udp_cl = 0;
              freeaddrinfo(udp_client.res);
              close(udp_client.fd);
              udp_client.fd = -1;
            }
            else fprintf(stdout, "I can't have the same key as my sucessor.\n");
          }
        }
      }
      else{
        fprintf(stderr, "Closing UDP Client connection.\n");
        freeaddrinfo(udp_client.res);
        close(udp_client.fd);
        state_udp_cl = 0;
        udp_client.fd = -1;
      }
    }


    /* WAITING FOR CONNECTING AS TCP SERVER*/
    if(FD_ISSET(tcp_server.fd, &rfds)){
      tcp_server.addrlen = sizeof(tcp_server.addr);
      if((newfd = accept(tcp_server.fd, (struct sockaddr*) &tcp_server.addr,
            &tcp_server.addrlen)) == -1) /*error*/ exit(1);
      if(!(my_data.state_sv)){
        afd = newfd;
        my_data.state_sv = 1;
        fprintf(stdout, "First connection done successfully.\n");
      }
      else if(!(my_data.state_new_conection)){
        new_conection_fd = newfd;
        my_data.state_new_conection = 1;
        fprintf(stdout, "Received new conection data.\n");
      }
      else{
      fprintf(stderr, "To many connecction to be processed.\n");
      close(newfd);
      }
    }


    /* WAITING TO READ AS TCP SERVER*/
    if(FD_ISSET(afd, &rfds)){
      //printf("Entrou?!\n");
      //O codigo está stuck no read devia estar a ler new
      memset(buffer, 0, MAX);
      if((tcp_server.n = read(afd, buffer, 128)) != 0){
        if(tcp_server.n == -1) /*error*/ exit(1);
        fprintf(stdout, "Received: %s", buffer);
        strcat(tcp_server.buffer, buffer);
        if(sscanf(tcp_server.buffer, "%*[^\n]s%c", &eol) == 1 && eol == '\n'){
          take_a_decision(&tcp_server, afd, tcp_client.fd, &my_data);
          /*memset(buffer, 0, MAX);
          if(sscanf(tcp_server.buffer, "%*[^\n]s%*c%[^\0]s", buffer) == 1 && eol == '\n'){
            memset(tcp_server.buffer, 0, MAX);
            strcat(tcp_server.buffer, buffer);
          }
          else*/ memset(tcp_server.buffer, 0, MAX);
        }
      }
      else{
        fprintf(stderr, "\nConnection lost with predecessor.\n");
        close(afd);
        afd = -1;
        my_data.state_sv = 0;
      }
    }


    /* WAITING TO READ AS TCP CLIENT */
    if(FD_ISSET(tcp_client.fd, &rfds)){
      memset(buffer, 0, MAX);
      if((tcp_client.n = read(tcp_client.fd, buffer, 128)) != 0){
        strcat(tcp_client.buffer, buffer);
        if(tcp_client.n == -1) /*error*/ exit(1);
        fprintf(stdout, "echo: %s", tcp_client.buffer);
        take_a_decision(&tcp_client, tcp_client.fd, afd, &my_data);
      }
      else{
        fprintf(stdout, "\nConnection lost with sucessor.\nEstablishing connection to new sucessor...\n");
        freeaddrinfo(tcp_client.res);
        close(tcp_client.fd);
        tcp_client.fd = -1;
        strcpy(my_data.succ_ip, my_data.s_succ_ip);
        strcpy(my_data.succ_gate, my_data.s_succ_gate);
        my_data.succ_key = my_data.s_succ_key;
        tcp_client = init_tcp_cl(my_data.succ_ip, my_data.succ_gate);
        if(my_data.succ_key != my_data.key){
          memset(msg, 0, MAX);
          sprintf(msg, "SUCC %d %s %s\n", my_data.succ_key, my_data.succ_ip, my_data.succ_gate);
          tcp_server.n = write(afd, msg, MAX);
          if(tcp_server.n == -1) /*error*/ exit(1);
          memset(msg, 0, MAX);
          sprintf(msg, "SUCCCONF\n");
          tcp_client.n = write(tcp_client.fd, msg, MAX);
          if(tcp_client.n == -1) /*error*/ exit(1);
        }
      }
    }


    /* WAITING TO READ AS NEW TCP SERVER*/
    if(FD_ISSET(new_conection_fd, &rfds)){
      //printf("Entrou!!\n");
      memset(buffer, 0, MAX);
      if((tcp_server.n = read(new_conection_fd, buffer, 128)) != 0){
        strcat(new_conection_buffer, buffer);
        if(tcp_server.n == -1) /*error*/ exit(1);
        afd = new_conection_to_me(afd, new_conection_fd, buffer, my_data, udp_server);
        my_data.asked_for_entry = 0;
        new_conection_fd = -1;
        my_data.state_new_conection = 0;
        fprintf(stderr, "New connection processed successfully.\n");
      }
      else{
        fprintf(stderr, "Connection lost with new_conection.\n");
        close(new_conection_fd);
        new_conection_fd = -1;
        my_data.state_new_conection = 0;
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


      /*ENTRY: adding a server in the ring without knowing it's location */
      else if(strcmp(token, "entry") == 0 && inside_a_ring == 0){
        if(sscanf(buffer, "%*s %d %d %s %s%c", &my_data.key, &entry_sv_key, entry_sv_ip, entry_sv_gate, &eol) == 5 && eol == '\n'){
          if (!(state_udp_cl)){
            udp_client = init_udp_cl(entry_sv_ip, entry_sv_gate);
            fprintf(stderr,"-> UDP connection done.\n");
            state_udp_cl = 1;
            memset(msg, 0, MAX);
            sprintf(msg,"EFND %d\n", my_data.key);
            udp_client.n = sendto(udp_client.fd, msg, strlen(msg), 0, udp_client.res->ai_addr, udp_client.res->ai_addrlen);
            if(udp_client.n == -1) /*error*/ exit(1);
            fprintf(stderr, "-> Sent message as udp client: %s", msg);
          }
          else{
            fprintf(stdout, "Already trying to connect.\n");
          }
      	}
				else{
        fprintf(stdout, "-> The command \\entry is of type \"entry i boot boot.ip boot.gate\". Where i is a key.\n");
        memset(buffer, 0, MAX);
        memset(token, 0, MAX);
        }
      }

      /*SENTRY: adding a server specifying it's successor */
      else if(strcmp(token, "sentry") == 0 && !(inside_a_ring)){
        if(sscanf(buffer, "%*s %d %d %s %s%c", &my_data.key, &my_data.succ_key, my_data.succ_ip, my_data.succ_gate, &eol) == 5 && eol == '\n'){
           if(sentry(&my_data, &tcp_client, msg) == 0 ){
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
        leave(&tcp_client, &afd, &my_data);
        inside_a_ring = 0;
      }

      /* FALTA ADICIONAR O ESTADO DO SERVIDOR!!! */
      else if(strcmp(buffer, "show\n") == 0){
        if(inside_a_ring)
          fprintf(stdout, "-> Key: %d\n-> IP: %s\n-> PORT: %s\n-> SuccKey: %d\n-> SuccIP: %s\n-> SuccPORT: %s\n-> S_SuccKey: %d\n-> S_SuccIP: %s\n-> S_SuccPORT: %s\n", my_data.key, my_data.ip, my_data.gate, my_data.succ_key, my_data.succ_ip, my_data.succ_gate, my_data.s_succ_key, my_data.s_succ_ip, my_data.s_succ_gate);
        else fprintf(stdout, "Not inside a ring, so I don't have a successor.\n");
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
        if(inside_a_ring) leave(&tcp_client, &afd, &my_data);
        free_program_data(my_data);
        fprintf(stderr, "Closing UDP Server connections.\n");
        freeaddrinfo(udp_server.res);
        close(udp_server.fd);
        freeaddrinfo(tcp_server.res);
        close(tcp_server.fd);
        fprintf(stderr, "\nExiting the application...\n");
        exit(EXIT_SUCCESS);
      }
      /*Invalid command, ignores it*/
      else fprintf(stderr, "ERROR -> Invalid command.\n");
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
  init_data.ip = calloc(MIN, sizeof(char));
  init_data.gate = calloc(MIN, sizeof(char));
  init_data.succ_ip = calloc(MIN, sizeof(char));
  init_data.succ_gate = calloc(MIN, sizeof(char));
  init_data.s_succ_ip = calloc(MIN, sizeof(char));
  init_data.s_succ_gate = calloc(MIN, sizeof(char));
  init_data.state_cl=0;
  init_data.state_sv=0;
  init_data.state_new_conection=0;
	init_data.asked_for_entry = 0;
  return init_data;
}

int free_program_data(struct Program_data free_data){
  free(free_data.s_succ_ip);
  free(free_data.s_succ_gate);
	if(free_data.succ_ip != free_data.s_succ_ip)
  	free(free_data.succ_ip);
  else fprintf(stderr, "There's proobably an error in the code!\n");
	if(free_data.succ_gate != free_data.s_succ_gate)
  	free(free_data.succ_gate);
  else fprintf(stderr, "There's proobably an error in the code!\n");
	if(free_data.ip != free_data.succ_ip && free_data.ip != free_data.s_succ_ip)
  	free(free_data.ip);
  else fprintf(stderr, "There's proobably an error in the code!\n");
	if(free_data.gate != free_data.succ_gate && free_data.gate != free_data.s_succ_gate)
  	free(free_data.gate);
  else fprintf(stderr, "There's proobably an error in the code!\n");
  return 0;
}

int leave(struct Program_connection* tcp_client, int* afd, struct Program_data* my_data){
  freeaddrinfo(tcp_client->res);
  close(tcp_client->fd);
  close(*afd);
  *afd = -1;
  tcp_client->fd = -1;
  my_data->state_cl = 0;
  my_data->state_sv = 0;
  my_data->state_new_conection = 0;
  fprintf(stderr, "Leaving the ring...\n");
  return 0;
}

int sentry(struct Program_data* my_data, struct Program_connection* tcp_client, char* msg){

  if(my_data->key == my_data->succ_key) return 1;

  *tcp_client = init_tcp_cl(my_data->succ_ip, my_data->succ_gate);
  my_data->state_cl = 1;
  memset(msg, 0, MAX);
  sprintf(msg, "NEW %d %s %s\n", my_data->key, my_data->ip, my_data->gate);
  tcp_client->n = write(tcp_client->fd, msg, MAX);
  if(tcp_client->n == -1) /*error*/ exit(1);

  fprintf(stdout, "-> Key: %d\n", my_data->key);
  fprintf(stdout, "-> Next server key: %d\n", my_data->succ_key);
  fprintf(stdout, "-> Next server ip: %s\n", my_data->succ_ip);
  fprintf(stdout, "-> Next server gate: %s\n", my_data->succ_gate);
  fprintf(stderr, "Server sentered...\n");
  return 0;
 }
