#include <netdb.h>

#define BACKLOG 10
#define MAX_BUF_ECHO_MSG 512
extern const char *default_port;

typedef struct {
  char *host;
  const char *port;
  int client_fd;
  int server_fd;
} conn_info;

extern conn_info conn_inf;

int get_tcp_client_sockfd(void);

int get_tcp_server_sockfd(void);

void assign_udp_server_fd(void);

void parse_server_opts(const int argc, char *argv[]);
void parse_client_opts(const int argc, char *argv[]);

void assign_tcp_dual_stack_client_fd(void);
void assign_tcp_dual_stack_server_fd(void);

void get_user_input(char *buffer, size_t size, const char *prompt);
