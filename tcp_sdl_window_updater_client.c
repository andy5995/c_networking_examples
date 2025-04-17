#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  struct socket_info_t socket_info;
  parse_client_opts(argc, argv, &socket_info);

  assign_tcp_dual_stack_client_fd(&socket_info);

  struct sdl_context_t sdl_context;
  init_sdl_window(&sdl_context, "SDL Client");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, socket_info.sockfd, SQUARE, &receiver);

  if (socket_info.sockfd != INVALID_SOCKET) {
    shutdown_socket_checked(socket_info.sockfd);
    close_socket_checked(socket_info.sockfd);
    pthread_join(receiver, NULL);
  }

  do_sdl_cleanup(&sdl_context);

  return 0;
}
