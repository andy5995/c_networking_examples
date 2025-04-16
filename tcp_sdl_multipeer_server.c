#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  struct connection conn_info;
  parse_server_opts(argc, argv, &conn_info);
  assign_tcp_dual_stack_server_fd(&conn_info);

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  socket_t client_fd = accept(conn_info.sockfd, (struct sockaddr *)&client_addr, &addr_size);
  if (conn_info.sockfd == INVALID_SOCKET) {
    perror("Client connection failed");
  }
  // server_fd only needed if more connections are desired
  close_socket_checked(conn_info.sockfd);

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Server");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, client_fd, CIRCLE, &receiver);

  if (client_fd != INVALID_SOCKET) {
    shutdown_socket_checked(client_fd);
    close_socket_checked(client_fd);
    pthread_join(receiver, NULL);
  }

  do_sdl_cleanup(&sdl_context);

  return 0;
}
