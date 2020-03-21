#ifndef HEADER_FILE
#define HEADER_FILE

struct Connection {
  int fd;
  int errcode;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *res;
  struct sockaddr_in addr;
  char buffer[128];
};

struct Connection init_udp_sv(char*);
struct Connection listen_udp_sv(struct Connection);
void close_udp_sv(struct Connection);

struct Connection init_tcp_sv(char*);
struct Connection listen_tcp_sv(struct Connection);
void close_tcp_sv(struct Connection);

struct Connection init_udp_cl(char*, char*);
struct Connection request_udp_cl(struct Connection, char*);
void close_udp_cl(struct Connection);

struct Connection init_tcp_cl(char*, char*);
struct Connection request_tcp_cl(struct Connection, char*);
void close_tcp_cl(struct Connection);

#endif
