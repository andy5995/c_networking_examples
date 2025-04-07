#include <SDL2/SDL.h>
#include <stdbool.h>

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480
#define CIRCLE_RADIUS 20

void draw_filled_area(SDL_Renderer *renderer, int x, int y, int r, bool circle);
