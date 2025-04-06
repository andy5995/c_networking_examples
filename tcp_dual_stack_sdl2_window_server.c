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
  int server_fd = fds[0];
  int *client_fd = &fds[1];

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

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("SDL2 Server", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  // Draw white background
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  int circle_x = WINDOW_WIDTH / 2, circle_y = WINDOW_HEIGHT / 2;

  draw_filled_circle(renderer, circle_x, circle_y, CIRCLE_RADIUS);
  SDL_RenderPresent(renderer);

  int fds[2] = {server_fd, -1};
  int first_packet_sent = 0;
  pthread_t net_thread;
  pthread_create(&net_thread, NULL, accept_thread, fds);

  int prev_circle_x = circle_x, prev_circle_y = circle_y;

  int running = 1;
  while (running) {
    if (fds[1] != -1 && (!first_packet_sent || prev_circle_x != circle_x ||
                         prev_circle_y != circle_y)) {
      char message[64];
      int len =
          snprintf(message, sizeof(message), "%d %d\n", circle_x, circle_y);

      send(fds[1], message, len, 0);
      if (!first_packet_sent)
        first_packet_sent = 1;
      prev_circle_x = circle_x;
      prev_circle_y = circle_y;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        circle_x = event.button.x;
        circle_y = event.button.y;
      }
    }

    // Draw white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    draw_filled_circle(renderer, circle_x, circle_y, CIRCLE_RADIUS);

    SDL_RenderPresent(renderer);
    SDL_Delay(16); // Small delay to prevent high CPU usage
  }

  if (fds[1] == -1) {
    if (shutdown(server_fd, SHUT_RDWR) != 0)
      perror("shutdown:");
  }
  pthread_join(net_thread, NULL);

  close(server_fd);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
