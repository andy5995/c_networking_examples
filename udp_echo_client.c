// Adapted from the getaddrinfo() man page
// https://linux.die.net/man/3/getaddrinfo
/*
 udp_echo_client.c
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
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s, j;
  socklen_t len;
  ssize_t nread;
  char buf[BUFSIZ];

  if (argc < 3)
  {
    fprintf (stderr, "Usage: %s host port msg...\n", argv[0]);
    return -1;
  }

  /* Obtain address(es) matching host/port */

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM;       /* Datagram socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;        /* Any protocol */

  s = getaddrinfo (argv[1], argv[2], &hints, &result);
  if (s != 0)
  {
    fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */

  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect (sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;                    /* Success */

    close (sfd);
  }

  if (rp == NULL)
  {                             /* No address succeeded */
    fputs ("Could not connect\n", stderr);
    return -1;
  }

  freeaddrinfo (result);        /* No longer needed */

  /* Send remaining command-line arguments as separate
     datagrams, and read responses from server */

  for (j = 3; j < argc; j++)
  {
    len = strlen (argv[j]) + 1;
    /* +1 for terminating null byte */

    if (len + 1 > BUFSIZ)
    {
      fprintf (stderr, "Ignoring long message in argument %d\n", j);
      continue;
    }

    if (write (sfd, argv[j], len) != len)
    {
      fputs ("partial/failed write\n", stderr);
      return -1;
    }

    nread = read (sfd, buf, BUFSIZ);
    if (nread == -1)
    {
      perror ("read");
      return -1;
    }

    printf ("Received %ld bytes: %s\n", (long) nread, buf);
  }

  return 0;
}
