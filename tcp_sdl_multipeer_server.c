#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  parse_server_opts(argc, argv);
  assign_tcp_dual_stack_server_fd();

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  conn_inf.client_fd =
      accept(conn_inf.server_fd, (struct sockaddr *)&client_addr, &addr_size);
  if (conn_inf.client_fd == -1) {
    perror("Client connection failed");
  }

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Server");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, conn_inf.client_fd, CIRCLE, &receiver);

  if (conn_inf.client_fd == -1) {
    if (shutdown(conn_inf.server_fd, SHUT_RDWR) != 0)
      perror("shutdown");
    if (close(conn_inf.client_fd) != 0)
      perror("close");
    pthread_join(receiver, NULL);
  }

  if (close(conn_inf.server_fd) != 0)
    perror("close");

  do_sdl_cleanup(&sdl_context);

  return 0;
}
