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

int create_conn(conn_info *conn_inf);
