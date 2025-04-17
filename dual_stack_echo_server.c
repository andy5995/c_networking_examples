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

  struct socket_info_t socket_info;
  parse_server_opts(argc, argv, &socket_info);
  assign_tcp_dual_stack_server_fd(&socket_info);

  while (1 && stop != 1) {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    socket_t connfd = accept(socket_info.sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (connfd == INVALID_SOCKET)
      continue;

    send(connfd, MESSAGE, strlen(MESSAGE), 0);
    close_socket_checked(connfd);
  }

  puts("Closing socket...");
  close_socket_checked(socket_info.sockfd);
  return 0;
}
