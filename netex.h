#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

extern char *default_host;
extern char *default_port;

typedef struct {
  char *host;
  char *port;
  int sockfd;
} conn_info;

int tcp_client_conn(conn_info *conn_inf);
