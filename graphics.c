#include "graphics.h"

void draw_filled_area(SDL_Renderer *renderer, int x, int y, int r, bool circle) {
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  for (int w = 0; w < r * 2; w++) {
    for (int h = 0; h < r * 2; h++) {
      int dx = r - w;
      int dy = r - h;
      if (circle) {
        if ((dx * dx + dy * dy) <= (r * r))
          SDL_RenderDrawPoint(renderer, x + dx, y + dy);
      }
      else
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);

    }
  }
  return;
}
