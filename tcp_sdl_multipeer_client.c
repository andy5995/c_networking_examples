#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Include for sscanf
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "graphics.h"
#include "netex.h"

int connect_to_server() {

  struct addrinfo hints, *res, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(conn_inf.host, conn_inf.port, &hints, &res) != 0) {
    perror("getaddrinfo");
    return -1;
  }

  for (p = res; p != NULL; p = p->ai_next) {
    conn_inf.sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (conn_inf.sockfd == -1)
      continue;

    if (connect(conn_inf.sockfd, p->ai_addr, p->ai_addrlen) == 0)
      break;
    close(conn_inf.sockfd);
  }

  freeaddrinfo(res);
  if (!p) {
    perror("Failed to connect");
    return -1;
  }

  return conn_inf.sockfd;
}

int main(int argc, char *argv[]) {
  parse_client_opts(argc, argv);

  conn_inf.sockfd = connect_to_server();
  if (conn_inf.sockfd == -1)
    return 1;

  struct sdl_context sdl_context;
  init_sdl_window(&sdl_context, "SDL Client");

  pthread_t receiver;
  run_sdl_loop(sdl_context.renderer, conn_inf.sockfd, SQUARE, &receiver);

  if (shutdown(conn_inf.sockfd, SHUT_RDWR) != 0)
    perror("shutdown:");

  if (close(conn_inf.sockfd) != 0)
    perror("close:");
  pthread_join(receiver, NULL);

  do_sdl_cleanup(&sdl_context);

  return 0;
}
