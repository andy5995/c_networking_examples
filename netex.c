#include <string.h>
#include <unistd.h>
#include "netex.h"

conn_info conn_inf = {
  "127.0.0.1",
  "8080",
  0
};

/* show_ip
 * only needed to demonstrate how to get and display the IP.
*/
static void
show_ip(struct addrinfo *rp)
{
  void *addr;
  char *ipver;
  char ipstr[INET6_ADDRSTRLEN];
  // get the pointer to the address itself,
  // different fields in IPv4 and IPv6:
  if (rp->ai_family == AF_INET)
  {                             // IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *) rp->ai_addr;
    addr = &(ipv4->sin_addr);
    ipver = "IPv4";
  }
  else
  {                             // IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) rp->ai_addr;
    addr = &(ipv6->sin6_addr);
    ipver = "IPv6";
  }

  // convert the IP to a string and print it:
  inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
  printf("  %s: %s\n", ipver, ipstr);

  return;
}

int
get_tcp_client_sockfd(void)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  /* Obtain address(es) matching host/port */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;        /* Any protocol */

  int s = getaddrinfo(conn_inf.host, conn_inf.port, &hints, &result);
  if (s != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */
  conn_inf.sockfd = -1;
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    show_ip(rp);
    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.sockfd == -1)
      continue;

    if (connect(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
    {
      printf("Connected to %s\n", conn_inf.host);
      break;
    }
    perror("connect");
    if (close(conn_inf.sockfd) != 0)
      perror("close");
    return -1;
  }

  freeaddrinfo(result);         /* No longer needed */

  if (conn_inf.sockfd == -1)
  {
    fputs("Unable to create socket\n", stderr);
    return -1;
  }
  return 0;
}

int get_udp_server_sockfd(void) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM;       /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;  /* For wildcard IP address */
  hints.ai_protocol = 0;        /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  int s = getaddrinfo(NULL, conn_inf.port, &hints, &result);
  if (s != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully bind(2).
     If socket(2) (or bind(2)) fails, we (close the socket
     and) try the next address. */
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    show_ip(rp);

    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.sockfd == -1)
      continue;

    if (bind(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break;                    /* Success */

    close(conn_inf.sockfd);
  }

  if (rp == NULL)
  {                             /* No address succeeded */
    fputs("Could not bind\n", stderr);
    return -1;
  }

  freeaddrinfo(result);         /* No longer needed */
  return 0;
}