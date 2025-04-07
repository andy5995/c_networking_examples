#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Include for sscanf
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int connect_to_server(const char *server_addr) {
  int client_fd;
  struct addrinfo hints, *res, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(server_addr, PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    return -1;
  }

  for (p = res; p != NULL; p = p->ai_next) {
    client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (client_fd == -1)
      continue;

    if (connect(client_fd, p->ai_addr, p->ai_addrlen) == 0)
      break;
    close(client_fd);
  }

  freeaddrinfo(res);
  if (!p) {
    perror("Failed to connect");
    return -1;
  }

  return client_fd;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <server_address>\n", argv[0]);
    return 1;
  }

  int client_fd = connect_to_server(argv[1]);
  if (client_fd == -1)
    return 1;

  struct sdl_objects sdl_objects;
  init_sdl_window(&sdl_objects, "SDL Client");

  int x = -1, y = -1; // Invalid initial position

  pthread_t receiver;
  run_sdl_loop(sdl_objects.renderer, x, y, client_fd, SQUARE, &receiver);

  if (shutdown(client_fd, SHUT_RDWR) != 0)
    perror("shutdown:");

  if (close(client_fd) != 0)
    perror("close:");
  pthread_join(receiver, NULL);

  SDL_DestroyRenderer(sdl_objects.renderer);
  SDL_DestroyWindow(sdl_objects.window);
  SDL_Quit();

  return 0;
}
