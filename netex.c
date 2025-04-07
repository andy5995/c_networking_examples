#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "netex.h"

conn_info conn_inf = {"127.0.0.1", "8080", 0, 0};

/* show_ip
 * only needed to demonstrate how to get and display the IP.
 */
static void show_ip(struct addrinfo *rp) {
  void *addr;
  char *ipver;
  char ipstr[INET6_ADDRSTRLEN];
  // get the pointer to the address itself,
  // different fields in IPv4 and IPv6:
  if (rp->ai_family == AF_INET) { // IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
    addr = &(ipv4->sin_addr);
    ipver = "IPv4";
  } else { // IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
    addr = &(ipv6->sin6_addr);
    ipver = "IPv6";
  }

  // convert the IP to a string and print it:
  inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
  printf("  %s: %s\n", ipver, ipstr);

  return;
}

int get_tcp_client_sockfd(void) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  /* Obtain address(es) matching host/port */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0; /* Any protocol */

  int s = getaddrinfo(conn_inf.host, conn_inf.port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */
  conn_inf.sockfd = -1;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    show_ip(rp);
    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.sockfd == -1)
      continue;

    if (connect(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      printf("Connected to %s\n", conn_inf.host);
      break;
    }
    perror("connect");
    if (close(conn_inf.sockfd) != 0)
      perror("close");
    return -1;
  }

  freeaddrinfo(result); /* No longer needed */

  if (conn_inf.sockfd == -1) {
    fputs("Unable to create socket\n", stderr);
    return -1;
  }
  return 0;
}

int get_tcp_server_sockfd(void) {
  struct sockaddr_in servaddr;

  conn_inf.sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (conn_inf.sockfd == -1) {
    perror("socket");
    return -1;
  } else
    puts("Socket successfully created");

  memset(&servaddr, 0, sizeof servaddr);

  servaddr.sin_family = AF_UNSPEC;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(conn_inf.port));

  // Binding newly created socket to given IP and verification
  if ((bind(conn_inf.sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) !=
      0) {
    perror("bind");
    close(conn_inf.sockfd);
    return -1;
  }

  printf("Socket successfully binded..\n");
  // Now server is ready to listen and verification
  if ((listen(conn_inf.sockfd, 5)) != 0) {
    perror("listen");
    close(conn_inf.sockfd);
    return -1;
  } else
    printf("Server listening on port %s...\n", conn_inf.port);
  return 0;
}

int get_udp_server_sockfd(void) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  int s = getaddrinfo(NULL, conn_inf.port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully bind(2).
     If socket(2) (or bind(2)) fails, we (close the socket
     and) try the next address. */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    show_ip(rp);

    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.sockfd == -1)
      continue;

    if (bind(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; /* Success */

    close(conn_inf.sockfd);
  }

  if (rp == NULL) { /* No address succeeded */
    fputs("Could not bind\n", stderr);
    return -1;
  }

  freeaddrinfo(result); /* No longer needed */
  return 0;
}

static void show_server_usage(const char *prgname) {
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  printf("\
  -a <host>\n\
  -p <port> (Default: %s)\n\n",
         PORT);
  return;
}

static void show_client_usage(const char *prgname) {
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  printf("\
  -p <port> (Default: %s)\n\n",
         PORT);
  return;
}

void parse_server_opts(const int argc, char *argv[], struct conn_info2 *x) {
  int opt;
  *x->host = '\0';
  *x->port = '\0';

  while ((opt = getopt(argc, argv, "p:h")) != -1) {
    switch (opt) {
    case 'p':
      conn_inf.port = optarg;
      break;
    case 'h':
    default:
      show_server_usage(argv[0]);
      exit(0);
    }
  }

  if (!*x->port)
    snprintf(x->port, NI_MAXSERV, "%s", PORT);

  return;
}

void parse_client_opts(const int argc, char *argv[], struct conn_info2 *x) {
  int opt;
  *x->host = '\0';
  *x->port = '\0';

  while ((opt = getopt(argc, argv, "a:p:h")) != -1) {
    switch (opt) {
    case 'p':
      snprintf(x->port, NI_MAXSERV, "%s", optarg);
      conn_inf.port = optarg;
      break;
    case 'a':
      snprintf(x->host, NI_MAXHOST, "%s", optarg);
      break;
    case 'h':
    default:
      show_client_usage(argv[0]);
      exit(EXIT_SUCCESS);
    }
  }

  if (!*x->port)
    snprintf(x->port, NI_MAXSERV, "%s", PORT);

  if (!*x->host) {
    fputs("-a <host> is required\n", stderr);
    exit(EXIT_FAILURE);
  }

  return;
}

int setup_tcp_dual_stack_server(void) {
  int server_fd;
  struct addrinfo hints, *res, *p;

  // Set up hints for dual-stack
  memset(&hints, 0, sizeof(hints));
  // Use IPv6, but allow IPv4 via v6-mapped addresses
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // Auto-fill IP

  // Get address info
  if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    return 1;
  }

  int optval = 0, opt = 1;
  // Create and bind socket
  for (p = res; p != NULL; p = p->ai_next) {
    server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (server_fd == -1)
      continue;

    if (setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval,
                   sizeof(optval)) < 0)
      perror("setsockopt IPV6_V6ONLY");

    // When starting the server immediately after it was killed,
    // prevent the error "Address already in use" when running
    // See https://linux.die.net/man/3/setsockopt for more information.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
      perror("setsockopt SO_REUSEADDR");

    if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0)
      break; // Success
    close(server_fd);
  }

  freeaddrinfo(res);
  if (!p) {
    perror("Failed to bind");
    return 1;
  }

  // Start listening
  if (listen(server_fd, BACKLOG) == -1) {
    perror("listen");
    return 1;
  }
  return server_fd;
}

#include <stdbool.h> // Make sure this is included

void *recv_thread(void *arg) {
  struct recv_args *args = (struct recv_args *)arg;
  char buffer[BUFFER_SIZE];
  while (1) {
    ssize_t bytes_received = recv(args->sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
      fputs("error: recv\n", stderr);
      if (bytes_received == -1)
        perror("recv:");
      break; // connection closed or error
    }

    buffer[bytes_received] = '\0';
    printf("received bytes: %s\n", buffer);
    int new_x, new_y;
    int new_circle_int; // Use int here for sscanf

    if (sscanf(buffer, "%d %d %d", &new_x, &new_y, &new_circle_int) == 3) {
      *args->x = new_x;
      *args->y = new_y;
      *args->received_first_update = 1;
      *args->circle = new_circle_int;
    }
  }
  return NULL;
}

