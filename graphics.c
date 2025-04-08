#include <pthread.h>

#include "graphics.h"

void init_sdl_window(struct sdl_objects *sdl_objects, const char *title)
{
  SDL_Init(SDL_INIT_VIDEO);
  const char *client = strstr(title, "Client");
  int win_pos_x = (client != NULL) ? WINDOW_WIDTH / 2 + 10 : SDL_WINDOWPOS_CENTERED;
  int win_pos_y = (client != NULL) ? WINDOW_HEIGHT / 2 + 10 : SDL_WINDOWPOS_CENTERED;
  sdl_objects->window = SDL_CreateWindow(title, win_pos_x, win_pos_y, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  sdl_objects->renderer =
      SDL_CreateRenderer(sdl_objects->window, -1, SDL_RENDERER_ACCELERATED);

  // Draw white background
  SDL_SetRenderDrawColor(sdl_objects->renderer, 255, 255, 255, 255);
  SDL_RenderClear(sdl_objects->renderer);
  SDL_RenderPresent(sdl_objects->renderer);
  return;
}

void draw_filled_area(SDL_Renderer *renderer, int x, int y, int r, enum e_shape shape) {
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  for (int w = 0; w < r * 2; w++) {
    for (int h = 0; h < r * 2; h++) {
      int dx = r - w;
      int dy = r - h;
      if (shape == CIRCLE) {
        if ((dx * dx + dy * dy) <= (r * r))
          SDL_RenderDrawPoint(renderer, x + dx, y + dy);
      }
      else
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);

    }
  }
  return;
}

void *recv_thread(void *arg) {
  struct peer_state *args = (struct peer_state *)arg;
  char buffer[BUFFER_SIZE];
  while (1) {
    ssize_t bytes_received = recv(args->sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
      if (bytes_received == -1)
        perror("recv:");
      break;
    }

    buffer[bytes_received] = '\0';
    printf("received bytes: %s\n", buffer);
    int new_x, new_y, new_shape;

    if (sscanf(buffer, "%d %d %d", &new_x, &new_y, &new_shape) == 3) {
      args->x = new_x;
      args->y = new_y;
      args->do_shape = new_shape;
    }
  }
  return NULL;
}

void run_sdl_loop(SDL_Renderer *renderer, int x, int y, int client_fd, enum e_shape shape, pthread_t *receiver)
{
  enum e_shape do_shape = shape;

  struct peer_state peer_state = {
      .sockfd = client_fd,
      .x = WINDOW_WIDTH / 2,
      .prev_x = 0,
      .y = WINDOW_HEIGHT /2,
      .prev_y = 0,
      .do_shape = shape,
  };

  pthread_create(receiver, NULL, recv_thread, &peer_state);
  int prev_x = x, prev_y = y;

  int running = 1;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        x = event.button.x;
        y = event.button.y;
        char message[64];
        int len = snprintf(message, sizeof(message), "%d %d %d\n", x, y, shape);
        if (send(client_fd, message, len, 0) == -1) {
          perror("send:");
          printf("client sending %s\n", message);
        }
      }
    }

    // Draw white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if (peer_state.prev_x != peer_state.x || peer_state.prev_y == peer_state.y) {
      peer_state.prev_x =  peer_state.x;
      peer_state.prev_y = peer_state.y;
      draw_filled_area(renderer, peer_state.x, peer_state.y, CIRCLE_RADIUS, peer_state.do_shape);
    }
    if (prev_x != x || prev_y == y) {
      prev_x = x;
      prev_y = y;
      draw_filled_area(renderer, x, y, CIRCLE_RADIUS, shape);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }
}
