#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "netex.h"

const char *default_port = "61357";

conn_info conn_inf = {
    .host = NULL, .port = NULL, .sockfd = -1, .server_fd = -1};

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

void assign_udp_server_fd() {
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

    conn_inf.server_fd =
        socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.server_fd == -1)
      continue;

    int optval = 0;
    if (setsockopt(conn_inf.server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval,
                   sizeof(optval)) < 0)
      perror("setsockopt IPV6_V6ONLY");

    if (bind(conn_inf.server_fd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; /* Success */

    close(conn_inf.server_fd);
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
      if (strlen(optarg) < NI_MAXSERV)
        conn_inf.port = optarg;
      else
        fprintf(stderr, "Port exceeds %d characters\n", NI_MAXSERV - 1);
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
      if (strlen(optarg) < NI_MAXSERV)
        conn_inf.port = optarg;
      else
        fprintf(stderr, "Port exceeds %d characters\n", NI_MAXSERV - 1);
      break;
    case 'a':
      if (strlen(optarg) < NI_MAXHOST)
        conn_inf.host = optarg;
      else
        fprintf(stderr, "Address (hostname) exceeds %d characters\n",
                NI_MAXHOST - 1);
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
    conn_inf.server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (conn_inf.server_fd == -1)
      continue;

    int optval = 0, opt = 1;
    if (setsockopt(conn_inf.server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval,
                   sizeof(optval)) < 0)
      perror("setsockopt IPV6_V6ONLY");

    // When starting the server immediately after it was killed,
    // prevent the error "Address already in use" when running
    // See https://linux.die.net/man/3/setsockopt for more information.
    if (setsockopt(conn_inf.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0)
      perror("setsockopt SO_REUSEADDR");

    if (bind(conn_inf.server_fd, p->ai_addr, p->ai_addrlen) == 0)
      break; // Success
    close(conn_inf.server_fd);
  }

  freeaddrinfo(res);
  if (!p) {
    perror("Failed to bind");
    exit(EXIT_FAILURE);
  }

  // Start listening
  if (listen(conn_inf.server_fd, BACKLOG) == -1) {
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
