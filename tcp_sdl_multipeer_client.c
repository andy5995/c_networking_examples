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
  run_sdl_loop(sdl_context.renderer, conn_inf.client_fd, SQUARE, &receiver);

  if (shutdown(conn_inf.client_fd, SHUT_RDWR) != 0)
    perror("shutdown");

  if (close(conn_inf.client_fd) != 0)
    perror("close");
  pthread_join(receiver, NULL);

  do_sdl_cleanup(&sdl_context);

  return 0;
}
