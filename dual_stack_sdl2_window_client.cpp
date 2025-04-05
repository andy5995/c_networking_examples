#include <iostream>
#include <sstream>              // Include for std::istringstream
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "dual_stack_sdl_window.h"
#include "graphics.h"

int
connect_to_server(const char *server_addr)
{
  int client_fd;
  struct addrinfo hints
  {
  }, *res, *p;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(server_addr, PORT, &hints, &res) != 0)
  {
    perror("getaddrinfo");
    return -1;
  }

  for (p = res; p != nullptr; p = p->ai_next)
  {
    client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (client_fd == -1)
      continue;

    if (connect(client_fd, p->ai_addr, p->ai_addrlen) == 0)
      break;
    close(client_fd);
  }

  freeaddrinfo(res);
  if (!p)
  {
    perror("Failed to connect");
    return -1;
  }

  return client_fd;
}

int
main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <server_address>\n";
    return 1;
  }

  int client_fd = connect_to_server(argv[1]);
  if (client_fd == -1)
    return 1;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("SDL2 Client", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer =
    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Draw white background
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  int x = -1, y = -1;           // Invalid initial position
  bool received_first_update = false;
  bool running = true;

  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
      }
    }

    // Receive coordinates from the server
    char buffer[BUFFER_SIZE] = { 0 };
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0)
    {
      buffer[bytes_received] = '\0';

      std::istringstream iss(buffer);
      if (iss >> x >> y)        // Safely extract values
      {
        received_first_update = true;   // Mark that we've received valid data
      }
      else
      {
        std::
          cerr << "Failed to parse server message: " << buffer << std::endl;
      }
    }

    // Draw white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Only draw the circle if we received a valid update from the server
    if (received_first_update)
    {
      draw_filled_circle(renderer, x, y, CIRCLE_RADIUS);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  close(client_fd);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
