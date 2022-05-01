#include <string.h>
#include <unistd.h>
#include "lib.h"

int
create_conn(conn_info *conn_inf)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  /* Obtain address(es) matching host/port */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;        /* Any protocol */

  int s = getaddrinfo(conn_inf->host, conn_inf->port, &hints, &result);
  if (s != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */
  conn_inf->sockfd = -1;
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    conn_inf->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf->sockfd == -1)
      continue;

    if (connect(conn_inf->sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
    {
      printf("Connected to %s\n", conn_inf->host);
      break;
    }
    perror("connect");
    if (close(conn_inf->sockfd) != 0)
      perror("close");
    return -1;
  }

  freeaddrinfo(result);         /* No longer needed */

  if (conn_inf->sockfd == -1)
  {
    fputs("Unable to create socket\n", stderr);
    return -1;
  }
  return 0;
}