#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream> // Include for std::istringstream
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "dual_stack_sdl_window.h"
#include "graphics.h"

int connect_to_server(const char *server_addr) {
  int client_fd;
  struct addrinfo hints{}, *res, *p;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(server_addr, PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    return -1;
  }

  for (p = res; p != nullptr; p = p->ai_next) {
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

void recv_thread(int client_fd, std::atomic<bool> *received_first_update,
                 std::atomic<int> *x, std::atomic<int> *y) {
  char buffer[BUFFER_SIZE];
  while (true) {
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0)
      break; // connection closed or error

    buffer[bytes_received] = '\0';
    std::istringstream iss(buffer);
    int new_x, new_y;
    if (iss >> new_x >> new_y) {
      *x = new_x;
      *y = new_y;
      *received_first_update = true;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <server_address>\n";
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

  std::atomic<int> x = 0, y = 0;
  std::atomic<bool> received_first_update = false;
  x = -1, y = -1; // Invalid initial position

  bool running = true;
  std::thread receiver(recv_thread, client_fd, &received_first_update, &x, &y);

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    // Draw white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Only draw the circle if we received a valid update from the server
    if (received_first_update) {
      draw_filled_circle(renderer, x, y, CIRCLE_RADIUS);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  if (receiver.joinable()) {
    if (shutdown(client_fd, SHUT_RDWR) != 0)
      perror("shutdown:");
    receiver.join();
  }
  close(client_fd);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
