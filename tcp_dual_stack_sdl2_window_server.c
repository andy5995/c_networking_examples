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

void *accept_thread(void *arg) {
  int *fds = (int *)arg;
  int server_fd = fds[SERVER_FD];
  int *client_fd = &fds[CLIENT_FD];

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
  if (*client_fd == -1) {
    perror("Client connection failed");
  }
  return NULL;
}

int main() {
  int server_fd = setup_tcp_dual_stack_server();
  if (server_fd == -1)
    return 1;

  printf("Server listening on port %s...\n", PORT);

  int fds[2] = {server_fd, -1};
  int first_packet_sent = 0;
  pthread_t net_thread;
  pthread_create(&net_thread, NULL, accept_thread, fds);
  pthread_join(net_thread, NULL);

  struct sdl_objects sdl_objects;
  init_sdl_window(&sdl_objects, "SDL Server");

  int x = WINDOW_WIDTH / 2, y = WINDOW_HEIGHT / 2;

  draw_filled_area(sdl_objects.renderer, x, y, CIRCLE_RADIUS, 1);
  SDL_RenderPresent(sdl_objects.renderer);

  pthread_t receiver;
  run_sdl_loop(sdl_objects.renderer, x, y, fds[CLIENT_FD], CIRCLE, &receiver);

  if (fds[CLIENT_FD] == -1) {
    if (shutdown(server_fd, SHUT_RDWR) != 0)
      perror("shutdown:");
    if (close(fds[CLIENT_FD]) != 0)
      perror("close:");
    pthread_join(receiver, NULL);
  }

  if (close(fds[SERVER_FD]) != 0)
    perror("close:");
  SDL_DestroyRenderer(sdl_objects.renderer);
  SDL_DestroyWindow(sdl_objects.window);
  SDL_Quit();

  return 0;
}
