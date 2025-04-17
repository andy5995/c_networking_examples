#include <pthread.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main(int argc, char *argv[]) {
  struct socket_info_t socket_info;
  parse_server_opts(argc, argv, &socket_info);
  assign_tcp_dual_stack_server_fd(&socket_info);

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  socket_t connfd = accept(socket_info.sockfd, (struct sockaddr *)&client_addr, &addr_size);
  if (socket_info.sockfd == INVALID_SOCKET) {
    perror("Client connection failed");
  }
  // server_fd only needed if more connections are desired
  close_socket_checked(socket_info.sockfd);

  struct sdl_context_t sdl_context;
  init_sdl_window(&sdl_context, "SDL Server");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, connfd, CIRCLE, &receiver);

  if (connfd != INVALID_SOCKET) {
    shutdown_socket_checked(connfd);
    close_socket_checked(connfd);
    pthread_join(receiver, NULL);
  }

  do_sdl_cleanup(&sdl_context);

  return 0;
}
