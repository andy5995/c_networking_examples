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

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("SDL2 Client", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Draw white background
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  int received_first_update = 0;
  int x = -1, y = -1; // Invalid initial position

  int prev_circle_x = x, prev_circle_y = y;
  int running = 1;

  int circle = 0;

  struct recv_args args = {
      .sockfd = client_fd,
      .x = &x,
      .y = &y,
      .received_first_update = &received_first_update,
      .circle = &circle,
  };

  pthread_t receiver;
  pthread_create(&receiver, NULL, recv_thread, &args);

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        x = event.button.x;
        y = event.button.y;
        circle = 0;
        char message[64];
        int len = snprintf(message, sizeof(message), "%d %d %d\n", x, y, circle);
        if (send(client_fd, message, len, 0) == -1)
          perror("send:");
        printf("client sending %s\n", message);
      }
    }

    // Draw white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Only draw the circle if we received a valid update from the server
    //if (received_first_update) {
      //draw_filled_area(renderer, x, y, CIRCLE_RADIUS, circle);
    //}

    if (prev_circle_x != x || prev_circle_y == y) {
      prev_circle_x = x;
      prev_circle_y = y;
      draw_filled_area(renderer, x, y, CIRCLE_RADIUS, circle);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  if (shutdown(client_fd, SHUT_RDWR) != 0)
    perror("shutdown:");
  pthread_join(receiver, NULL);
  close(client_fd);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
