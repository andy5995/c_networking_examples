/*
 * dual_stack_echo_server.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"

#define MESSAGE "hello world\n"

void handle_client(int client_fd) {
  send(client_fd, MESSAGE, strlen(MESSAGE), 0);
  close(client_fd);
}

int main(int argc, char *argv[]) {
  parse_server_opts(argc, argv);
  assign_tcp_dual_stack_server_fd();

  while (1) {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    conn_inf.client_fd = accept(conn_inf.server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (!IS_VALID_SOCKET(conn_inf.client_fd))
      continue;

    handle_client(conn_inf.client_fd);
  }

  close(conn_inf.server_fd);
  return 0;
}
