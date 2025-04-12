#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton, inet_ntop
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"

const char *default_port = "61357";

conn_info conn_inf = {.host = NULL, .port = NULL, .sockfd = INVALID_SOCKET, .client_fd = INVALID_SOCKET};

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

void assign_tcp_client_fd(void) {
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
    exit(EXIT_FAILURE);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    show_ip(rp);
    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (!IS_VALID_SOCKET(conn_inf.sockfd))
      continue;

    if (connect(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      printf("Connected to %s\n", conn_inf.host);
      break;
    }
    perror("connect");
    close_socket_checked(conn_inf.sockfd);

    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result); /* No longer needed */

  if (conn_inf.sockfd == INVALID_SOCKET) {
    fputs("Unable to create socket\n", stderr);
    exit(EXIT_FAILURE);
  }
  return;
}

void assign_tcp_server_fd(void) {
  struct sockaddr_in servaddr;

  conn_inf.sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (conn_inf.sockfd == INVALID_SOCKET) {
    perror("socket");
    exit(EXIT_FAILURE);
  } else {
    puts("Socket successfully created");
    set_sock_reuse();
  }

  memset(&servaddr, 0, sizeof servaddr);

  servaddr.sin_family = AF_UNSPEC;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(conn_inf.port));

  // Binding newly created socket to given IP and verification
  if ((bind(conn_inf.sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) {
    perror("bind");
    close(conn_inf.sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Socket successfully binded..\n");
  // Now server is ready to listen and verification
  if ((listen(conn_inf.sockfd, 5)) != 0) {
    perror("listen");
    close_socket_checked(conn_inf.sockfd);
    exit(EXIT_FAILURE);
  } else
    printf("Server listening on port %s...\n", conn_inf.port);
  return;
}

void assign_udp_server_fd(void) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;     /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  int s = getaddrinfo(conn_inf.host, conn_inf.port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully bind(2).
     If socket(2) (or bind(2)) fails, we (close the socket
     and) try the next address. */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    show_ip(rp);

    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.sockfd == INVALID_SOCKET)
      continue;

    set_sock_ipv6_v6only_disable(rp->ai_family);

    if (bind(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; /* Success */

    close(conn_inf.sockfd);
  }

  if (rp == NULL) { /* No address succeeded */
    fputs("Could not bind\n", stderr);
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result); /* No longer needed */
  return;
}

static void show_server_usage(const char *prgname) {
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  printf("\
  -p <port> (Optional; Default: %s)\n\n",
         default_port);
  return;
}

static void show_client_usage(const char *prgname) {
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  printf("\
  -a <host>\n\
  -p <port> (Optional; Default: %s)\n\n",
         default_port);
  return;
}

void parse_server_opts(const int argc, char *argv[]) {
  conn_inf.port = default_port;
  int opt;

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

  return;
}

void parse_client_opts(const int argc, char *argv[]) {
  conn_inf.port = default_port;
  int opt;

  while ((opt = getopt(argc, argv, "a:p:h")) != -1) {
    switch (opt) {
    case 'p':
      conn_inf.port = optarg;
      break;
    case 'a':
      conn_inf.host = optarg;
      break;
    case 'h':
    default:
      show_client_usage(argv[0]);
      exit(EXIT_SUCCESS);
    }
  }

  if (!conn_inf.host) {
    fputs("-a <host> is required\n", stderr);
    exit(EXIT_FAILURE);
  }

  return;
}

void assign_tcp_dual_stack_client_fd(void) {
  struct addrinfo hints, *res, *p;

  // Set up hints for getaddrinfo()
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // Allow both IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM;

  // Get address info
  if (getaddrinfo(conn_inf.host, conn_inf.port, &hints, &res) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  // Try to connect to one of the results
  for (p = res; p != NULL; p = p->ai_next) {
    conn_inf.sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (!IS_VALID_SOCKET(conn_inf.sockfd))
      continue;

    if (connect(conn_inf.sockfd, p->ai_addr, p->ai_addrlen) == 0)
      break; // Connected successfully

    close(conn_inf.sockfd);
  }

  freeaddrinfo(res);

  if (!p) {
    perror("Failed to connect");
    exit(EXIT_FAILURE);
  }
  return;
}

void assign_tcp_dual_stack_server_fd(void) {

  struct addrinfo hints, *res, *p;

  // Set up hints for dual-stack
  memset(&hints, 0, sizeof(hints));
  // Use IPv6, but allow IPv4 via v6-mapped addresses
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // Auto-fill IP

  // Get address info
  if (getaddrinfo(conn_inf.host, conn_inf.port, &hints, &res) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  // Create and bind socket
  for (p = res; p != NULL; p = p->ai_next) {
    conn_inf.sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (!IS_VALID_SOCKET(conn_inf.sockfd))
      continue;

    set_sock_reuse();
    set_sock_ipv6_v6only_disable(p->ai_family);

    if (bind(conn_inf.sockfd, p->ai_addr, p->ai_addrlen) == 0)
      break; // Success
    close(conn_inf.sockfd);
  }

  freeaddrinfo(res);
  if (!p) {
    perror("Failed to bind");
    exit(EXIT_FAILURE);
  }

  // Start listening
  if (listen(conn_inf.sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %s...\n", conn_inf.port);
  return;
}

void get_user_input(char *buffer, size_t size, const char *prompt) {
  if (prompt) {
    printf("%s", prompt);
    fflush(stdout);
  }

  if (fgets(buffer, size, stdin)) {
    // Remove trailing newline if it exists
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
  } else {
    // fgets failed — clear buffer
    buffer[0] = '\0';
  }
}

void set_sock_reuse(void) {
  int r = -1;
#ifdef _WIN32
  // When starting the server immediately after it was killed,
  // prevent the error "Address already in use" when running
  // See https://linux.die.net/man/3/setsockopt for more information.
  const char opt = 1;
  r = (setsockopt(conn_inf.sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0);

#else
  int opt = 1;
  r = (setsockopt(conn_inf.sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0);

#endif
  if (r == -1)
    perror("setsockopt SO_REUSEADDR");
}

void set_sock_ipv6_v6only_disable(const int ai_family) {
  int r = -1;
#ifdef _WIN32
  if (ai_family == AF_INET6) {
    const char optval = 0;
    r = (setsockopt(conn_inf.sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) < 0);
  }
#else
  (void)ai_family;
  int optval = 0;
  r = (setsockopt(conn_inf.sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) < 0);
#endif
  if (r == -1)
    perror("setsockopt IPV6_V6ONLY");
}

void shutdown_socket_checked(socket_t sockfd) {
  if (IS_VALID_SOCKET(sockfd)) {
#ifdef _WIN32
    if (shutdown(sockfd, SD_BOTH) == SOCKET_ERROR)
      perror("shutdown");
#else
    if (shutdown(sockfd, SHUT_RDWR) != 0)
      perror("shutdown");
#endif
  }
}

void close_socket_checked(socket_t sockfd) {
#ifdef _WIN32
  if (closesocket(sockfd) != 0)
    perror("closesocket");
#else
  if (close(sockfd) != 0)
    perror("close");
#endif
}
