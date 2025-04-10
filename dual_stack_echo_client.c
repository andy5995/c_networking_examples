/*
 * dual_stack_echo_client.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"

int main(int argc, char *argv[]) {
  parse_client_opts(argc, argv);
  assign_tcp_dual_stack_client_fd();

  char buffer[1024];
  // Read response from server
  ssize_t bytes_received = recv(conn_inf.client_fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received > 0) {
    buffer[bytes_received] = '\0'; // Null-terminate received data
    printf("Server response: %s", buffer);
  } else {
    perror("recv");
  }

  close(conn_inf.client_fd);
  return 0;
}
