#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Include for snprintf
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int main() {
  int server_fd = setup_tcp_dual_stack_server();
  if (server_fd == -1)
    return 1;

  printf("Server listening on port %s...\n", PORT);

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  int client_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
  if (client_fd == -1) {
    perror("Client connection failed");
  }

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Server");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, client_fd, CIRCLE, &receiver);

  if (client_fd == -1) {
    if (shutdown(server_fd, SHUT_RDWR) != 0)
      perror("shutdown:");
    if (close(client_fd) != 0)
      perror("close:");
    pthread_join(receiver, NULL);
  }

  if (close(server_fd) != 0)
    perror("close:");

  do_sdl_cleanup(&sdl_context);

  return 0;
}
