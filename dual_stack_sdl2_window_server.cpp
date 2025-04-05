#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream> // Include for std::ostringstream
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "dual_stack_sdl_window.h"
#include "graphics.h"

void accept_thread(int server_fd, int *client_fd) {
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
  if (*client_fd == -1) {
    perror("Client connection failed");
  }
  return;
}

// Function to set up the server socket
int setup_server_socket() {
  int server_fd;
  struct addrinfo hints{}, *res, *p;

  hints.ai_family = AF_INET6; // IPv6, supports v4 via v6-mapped addresses
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // Auto-fill IP

  if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    return -1;
  }

  int optval = 0;
  int opt = 1;
  for (p = res; p != NULL; p = p->ai_next) {
    server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (server_fd == -1)
      continue;

    if (setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval,
                   sizeof(optval)) < 0)
      perror("setsockopt IPV6_V6ONLY");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
      perror("setsockopt SO_REUSEADDR");

    if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0)
      break;
    close(server_fd);
  }

  freeaddrinfo(res);
  if (!p) {
    perror("Failed to bind");
    return -1;
  }

  if (listen(server_fd, BACKLOG) == -1) {
    perror("listen");
    return -1;
  }

  return server_fd;
}

int main() {
  int server_fd = setup_server_socket();
  if (server_fd == -1)
    return 1;

  std::cout << "Server listening on port " << PORT << "...\n";

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

  int client_fd = -1;
  bool first_packet_sent = false;
  std::thread net_thread(accept_thread, server_fd, &client_fd);

  int prev_circle_x = circle_x, prev_circle_y = circle_y;

  bool running = true;
  while (running) {
    if (client_fd != -1 && (!first_packet_sent || prev_circle_x != circle_x ||
                            prev_circle_y != circle_y)) {
      std::ostringstream oss;
      oss << circle_x << " " << circle_y << "\n";
      std::string message = oss.str();

      send(client_fd, message.c_str(), message.size(), 0);
      if (!first_packet_sent)
        first_packet_sent = true;
      prev_circle_x = circle_x;
      prev_circle_y = circle_y;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
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

  if (net_thread.joinable()) {
    if (client_fd == -1) {
      if (shutdown(server_fd, SHUT_RDWR) != 0)
        perror("shutdown:");
    }
    net_thread.join();
  }

  close(server_fd);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
