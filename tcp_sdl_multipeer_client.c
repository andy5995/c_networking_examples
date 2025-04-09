#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Include for sscanf
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  parse_client_opts(argc, argv);

  assign_tcp_dual_stack_client_fd();

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Client");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, conn_inf.sockfd, SQUARE, &receiver);

  if (shutdown(conn_inf.sockfd, SHUT_RDWR) != 0)
    perror("shutdown:");

  if (close(conn_inf.sockfd) != 0)
    perror("close:");
  pthread_join(receiver, NULL);

  do_sdl_cleanup(&sdl_context);

  return 0;
}
