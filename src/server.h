#ifndef HEADER_FILE
#define HEADER_FILE

#define MAX 128
#define MIN 32


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
  char buffer[128];
  int state_cl;
  int state_sv;
  int state_new_conection;
  int asked_for_entry;
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
int new_conection_to_me(int, int, char*, struct Program_data, struct Program_connection);
int take_a_decision(struct Program_connection*, int, int, struct Program_data*);

int max(int, int);
struct Program_data init_program_data();
int free_program_data(struct Program_data*);
int leave(struct Program_connection, int, struct Program_data*);
int sentry(struct Program_data*, struct Program_connection*, char*);

#endif
