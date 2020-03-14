#ifndef HEADER_FILE
#define HEADER_FILE

struct Client {
  int fd;
  int errcode;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *res;
  struct sockaddr_in addr;
  char buffer[128];
};

struct Client init_udp_cl(char*, char*);
struct Client listen_udp_cl(struct Server);
void close_udp_cl(struct Client);

struct Client init_tcp_cl(char*, char*);
struct Client listen_tcp_cl(struct Client);
void close_tcp_cl(struct Client);

#endif
