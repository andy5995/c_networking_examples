#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

typedef struct {
  char *host;
  char *port;
  int sockfd;
} conn_info;

extern conn_info conn_inf;

int get_tcp_client_sockfd(void);

int get_udp_server_sockfd(void);
