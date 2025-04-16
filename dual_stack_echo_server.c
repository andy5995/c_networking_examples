/*
 * dual_stack_echo_server.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"
#include "util.h"

#define MESSAGE "hello world\n"

int main(int argc, char *argv[]) {
#ifdef _WIN32
  if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
    fprintf(stderr, "Error setting console handler\n");
    return 1;
  }
#else
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
#endif

  struct connection conn_info;
  parse_server_opts(argc, argv, &conn_info);
  assign_tcp_dual_stack_server_fd(&conn_info);

  while (1 && stop != 1) {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    socket_t client_fd = accept(conn_info.sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == INVALID_SOCKET)
      continue;

    send(client_fd, MESSAGE, strlen(MESSAGE), 0);
    close_socket_checked(client_fd);
  }

  puts("Closing socket...");
  close_socket_checked(conn_info.sockfd);
  return 0;
}
