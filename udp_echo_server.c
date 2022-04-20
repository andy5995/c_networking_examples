// Adapted from the getaddrinfo() man page
// https://linux.die.net/man/3/getaddrinfo
/*
 udp_echo_server.c
 https://github.com/andy5995/c_networking_examples

 MIT License

 Copyright (c) 2022 Andy Alt and James Sherratt

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

/* show_ip
 * only needed to demonstrate how to get and display the IP.
*/
static void
show_ip (struct addrinfo *rp)
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
  inet_ntop (rp->ai_family, addr, ipstr, sizeof ipstr);
  printf ("  %s: %s\n", ipver, ipstr);

  return;
}

int
main (int argc, char *argv[])
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s;
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len;
  ssize_t nread;
  char buf[BUFSIZ];

  if (argc != 2)
  {
    fprintf (stderr, "Usage: %s port\n", argv[0]);
    return -1;
  }

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM;       /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;  /* For wildcard IP address */
  hints.ai_protocol = 0;        /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo (NULL, argv[1], &hints, &result);
  if (s != 0)
  {
    fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully bind(2).
     If socket(2) (or bind(2)) fails, we (close the socket
     and) try the next address. */
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    show_ip (rp);

    sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (bind (sfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break;                    /* Success */

    close (sfd);
  }

  if (rp == NULL)
  {                             /* No address succeeded */
    fputs ("Could not bind\n", stderr);
    return -1;
  }

  freeaddrinfo (result);        /* No longer needed */

  /* Read datagrams and echo them back to sender */
  for (;;)
  {
    peer_addr_len = sizeof (struct sockaddr_storage);
    nread = recvfrom (sfd, buf, BUFSIZ, 0,
                      (struct sockaddr *) &peer_addr, &peer_addr_len);
    if (nread == -1)
      continue;                 /* Ignore failed request */

    char host[NI_MAXHOST], service[NI_MAXSERV];

    s = getnameinfo ((struct sockaddr *) &peer_addr,
                     peer_addr_len, host, NI_MAXHOST,
                     service, NI_MAXSERV, NI_NUMERICSERV);
    if (s == 0)
      printf ("Received %ld bytes from %s:%s\n", (long) nread, host, service);
    else
      fprintf (stderr, "getnameinfo: %s\n", gai_strerror (s));

    ssize_t r_sto =
      sendto (sfd, buf, nread, 0, (struct sockaddr *) &peer_addr,
              peer_addr_len);

    if (strncasecmp (buf, "exit", 4) == 0)
    {
      puts ("Received 'exit'");
      close (sfd);
      return 0;
    }

    if (r_sto != nread)
    {
      fputs ("Error sending response\n", stderr);
      close (sfd);
      return r_sto;
    }
  }
  return 0;
}
