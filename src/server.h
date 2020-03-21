#ifndef HEADER_FILE
#define HEADER_FILE

#define MAX 100

struct Program_data{
  int key;
  int succ_key;
  char* succ_ip;
  succ_ip = malloc((MAX+1)*sizeof(char));
  char* succ_gate;
  succ_gate = malloc((MAX+1)*sizeof(char));
  char* s_succ_ip;
  s_succ_ip = malloc((MAX+1)*sizeof(char));
  char* s_succ_gate;
  s_succ_gate = malloc((MAX+1)*sizeof(char));
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

struct Connection init_tcp_sv(char*);
struct Program_connection init_tcp_cl(char*, char*);
int take_a_decision(struct Program_connection, int, struct Program_data);


#endif
