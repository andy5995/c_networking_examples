#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  parse_client_opts(argc, argv);

  assign_tcp_dual_stack_client_fd();

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Client");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, SQUARE, &receiver);

  if (IS_VALID_SOCKET(conn_inf.sockfd)) {
    shutdown_socket_checked(conn_inf.sockfd);
    close_socket_checked(conn_inf.sockfd);
    pthread_join(receiver, NULL);
  }

  do_sdl_cleanup(&sdl_context);

  return 0;
}
