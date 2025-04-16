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
  struct socket_info_t socket_info;
  parse_client_opts(argc, argv, &socket_info);
  assign_tcp_dual_stack_client_fd(&socket_info);

  char buffer[1024];
  // Read response from server
  ssize_t bytes_received = recv(socket_info.sockfd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received > 0) {
    buffer[bytes_received] = '\0';
    printf("Server response: %s", buffer);
  } else {
    perror("recv");
  }

  close_socket_checked(socket_info.sockfd);
  return 0;
}
