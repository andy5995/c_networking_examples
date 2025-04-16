#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  struct connection conn_info;
  parse_client_opts(argc, argv, &conn_info);

  assign_tcp_dual_stack_client_fd(&conn_info);

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Client");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, conn_info.sockfd, SQUARE, &receiver);

  if (conn_info.sockfd != INVALID_SOCKET) {
    shutdown_socket_checked(conn_info.sockfd);
    close_socket_checked(conn_info.sockfd);
    pthread_join(receiver, NULL);
  }

  do_sdl_cleanup(&sdl_context);

  return 0;
}
