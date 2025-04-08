#include <netdb.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480
#define CIRCLE_RADIUS 20
#define BUFFER_SIZE 64

enum e_shape {
  CIRCLE,
  SQUARE,
};

struct peer_state {
  int sockfd;
  int x;
  int prev_x;
  int y;
  int prev_y;
  enum e_shape do_shape;
};

struct sdl_objects {
  SDL_Renderer *renderer;
  SDL_Window *window;
};

void init_sdl_window(struct sdl_objects *sdl_objects, const char *title);

void draw_filled_area(SDL_Renderer *renderer, int x, int y, int r, enum e_shape shape);

void run_sdl_loop(SDL_Renderer *renderer, int x, int y, int client_fd, enum e_shape shape, pthread_t *receiver);

void *recv_thread(void *arg);
