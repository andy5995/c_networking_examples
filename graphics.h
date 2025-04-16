#include <SDL2/SDL.h>

#include "netex.h"

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480
#define CIRCLE_RADIUS 20

enum shape_t {
  CIRCLE,
  SQUARE,
};

struct sdl_context_t {
  SDL_Renderer *renderer;
  SDL_Window *window;
};

void init_sdl_window(struct sdl_context_t *sdl_context, const char *title);

void run_sdl_loop(SDL_Renderer *renderer, const socket_t sockfd, const enum shape_t shape, pthread_t *receiver);

void do_sdl_cleanup(struct sdl_context_t *sdl_context);
