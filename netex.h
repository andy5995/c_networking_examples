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

int get_tcp_client_sockfd(void);

int get_tcp_server_sockfd(void);

int get_udp_server_sockfd(void);

void parse_server_opts(const int argc, char *argv[]);

int setup_tcp_dual_stack_server();
