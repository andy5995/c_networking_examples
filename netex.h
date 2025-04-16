#pragma once

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#endif

#ifdef _WIN32
typedef SOCKET socket_t;
typedef int socklen_t;
#else
typedef int socket_t;
#define INVALID_SOCKET -1
#endif

struct connection {
  char *host;
  const char *port;
  socket_t sockfd;
};

#include "util.h"

#define BACKLOG 10
#define MAX_BUF_ECHO_MSG 512
extern const char *default_port;

void assign_tcp_client_fd(struct connection *conn_info);

void assign_tcp_server_fd(struct connection *conn_info);

void assign_udp_server_fd(struct connection *conn_info);

void assign_tcp_dual_stack_client_fd(struct connection *conn_info);
void assign_tcp_dual_stack_server_fd(struct connection *conn_info);

void set_sock_reuse(socket_t sockfd);

void set_sock_ipv6_v6only_disable(socket_t sockfd, const int ai_family);

void shutdown_socket_checked(socket_t sockfd);

void close_socket_checked(socket_t sockfd);
