#ifndef HEADER_FILE
#define HEADER_FILE

struct Server {
  int fd;
  int newfd;
  int errcode;
  int key;
  char succ_ip[20];
  char succ_gate[20];
  char s_succ_ip[20];
  char s_succ_gate[20];
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *res;
  struct sockaddr_in addr;
  char buffer[128];
};

struct Server init_udp_sv(char*);
struct Server listen_udp_sv(struct Server);
void close_udp_sv(struct Server);

struct Server init_tcp_sv(char*);
struct Server listen_tcp_sv(struct Server);
void close_tcp_sv(struct Server);

struct Client init_udp_cl(char*, char*);
struct Client request_udp_cl(struct Server);
void close_udp_cl(struct Client);

struct Client init_tcp_cl(char*, char*);
struct Client request_tcp_cl(struct Client);
void close_tcp_cl(struct Client);

#endif
