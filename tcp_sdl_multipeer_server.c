#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  parse_server_opts(argc, argv);
  assign_tcp_dual_stack_server_fd();

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  conn_inf.client_fd = accept(conn_inf.server_fd, (struct sockaddr *)&client_addr, &addr_size);
  if (!IS_VALID_SOCKET(conn_inf.client_fd)) {
    perror("Client connection failed");
  }
  // server_fd only needed if more connections are desired
  close_socket_checked(conn_inf.server_fd);

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Server");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, CIRCLE, &receiver);

  if (IS_VALID_SOCKET(conn_inf.client_fd)) {
    shutdown_socket_checked(conn_inf.client_fd);
    close_socket_checked(conn_inf.client_fd);
    pthread_join(receiver, NULL);
  }

  do_sdl_cleanup(&sdl_context);

  return 0;
}
