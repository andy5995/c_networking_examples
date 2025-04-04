#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory>

constexpr char PORT[] = "12345";
constexpr int BACKLOG = 10;
constexpr char MESSAGE[] = "hello world\n";

void
handle_client(int client_fd)
{
  send(client_fd, MESSAGE, std::strlen(MESSAGE), 0);
  close(client_fd);
}

int
main()
{
  int server_fd;
  struct addrinfo hints
  {
  }, *res, *p;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;   // Use IPv6, allow IPv4 via v6-mapped addresses
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(nullptr, PORT, &hints, &res) != 0)
  {
    std::cerr << "getaddrinfo failed" << std::endl;
    return 1;
  }

  int optval = 0;
  for (p = res; p != nullptr; p = p->ai_next)
  {
    server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (server_fd == -1)
      continue;

    setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));

    if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0)
      break;
    close(server_fd);
  }

  freeaddrinfo(res);
  if (!p)
  {
    std::cerr << "Failed to bind" << std::endl;
    return 1;
  }

  if (listen(server_fd, BACKLOG) == -1)
  {
    std::cerr << "listen failed" << std::endl;
    return 1;
  }

  std::cout << "Server listening on port " << PORT << "..." << std::endl;

  while (true)
  {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd =
      accept(server_fd, (struct sockaddr *) &client_addr, &addr_size);
    if (client_fd == -1)
      continue;

    handle_client(client_fd);
  }

  close(server_fd);
  return 0;
}
