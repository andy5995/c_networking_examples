#include "graphics.h"

// Function to draw a filled circle
void
draw_filled_circle(SDL_Renderer *renderer, int x, int y, int r)
{
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  for (int w = 0; w < r * 2; w++)
  {
    for (int h = 0; h < r * 2; h++)
    {
      int dx = r - w;
      int dy = r - h;
      if ((dx * dx + dy * dy) <= (r * r))
      {
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
      }
    }
  }
  return;
}
