#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#endif

#define BACKLOG 10
#define MAX_BUF_ECHO_MSG 512
extern const char *default_port;

#ifdef _WIN32
typedef SOCKET socket_t;
typedef int socklen_t;
#define IS_VALID_SOCKET(s) ((s) != INVALID_SOCKET)
#else
typedef int socket_t;
#define IS_VALID_SOCKET(s) ((s) >= 0)
#define INVALID_SOCKET -1
#endif


typedef struct {
  char *host;
  const char *port;
  socket_t sockfd;
  socket_t client_fd;
} conn_info;

extern conn_info conn_inf;

void assign_tcp_client_fd(void);

void assign_tcp_server_fd(void);

void assign_udp_server_fd(void);

void parse_server_opts(const int argc, char *argv[]);
void parse_client_opts(const int argc, char *argv[]);

void assign_tcp_dual_stack_client_fd(void);
void assign_tcp_dual_stack_server_fd(void);

void get_user_input(char *buffer, size_t size, const char *prompt);

void set_sock_reuse(void);

void set_sock_ipv6_v6only_disable(const int ai_family);

void shutdown_socket_checked(socket_t sockfd);

void close_socket_checked(socket_t sockfd);
