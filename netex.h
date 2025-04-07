#include <netdb.h>
#include <stdbool.h>

#define PORT "61357"
#define BUFFER_SIZE 64
#define BACKLOG 10

typedef struct {
  char *host;
  char *port;
  int sockfd;
  int connfd;
} conn_info;

extern conn_info conn_inf;

struct conn_info2 {
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  int sockfd;
};

struct recv_args {
  int sockfd;
  int *x;
  int *y;
  int *received_first_update;
  int *circle;
};

int get_tcp_client_sockfd(void);

int get_tcp_server_sockfd(void);

int get_udp_server_sockfd(void);

void parse_server_opts(const int argc, char *argv[], struct conn_info2 *x);
void parse_client_opts(const int argc, char *argv[], struct conn_info2 *x);

int setup_tcp_dual_stack_server();

void *recv_thread(void *arg);
