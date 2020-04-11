#ifndef HEADER_FILE
#define HEADER_FILE

#define MAX 100

struct Program_data{
  int key;
  int succ_key;
  int s_succ_key;
  char* ip;
  char* gate;
  char* succ_ip;
  char* succ_gate;
  char* s_succ_ip;
  char* s_succ_gate;
};

struct Program_connection {
  int fd;
  int errcode;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *res;
  struct sockaddr_in addr;
  char buffer[128];
};

struct Program_connection init_udp_sv(char*);
struct Program_connection listen_udp_sv(struct Program_connection);
void close_udp_sv(struct Program_connection);
struct Program_connection init_udp_cl(char*, char*);
struct Program_connection request_udp_cl(struct Program_connection, char*);
void close_udp_cl(struct Program_connection);


struct Program_connection init_tcp_sv(char*);
struct Program_connection init_tcp_cl(char*, char*);
int new_conection_to_me(int, int, struct Program_data);
int take_a_decision(struct Program_connection, int, int, struct Program_data*);

struct Program_data init_program_data();
int free_program_data(struct Program_data);
int leave(struct Program_connection, int, int*, int*);

#endif
