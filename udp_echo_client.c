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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"

int main(int argc, char *argv[]) {
  parse_client_opts(argc, argv);

  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s;
  ssize_t nread;
  socklen_t max_msg_size = 1024;
  char buf[max_msg_size];

  /* Obtain address(es) matching host/port */

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0; /* Any protocol */

  s = getaddrinfo(conn_inf.host, conn_inf.port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    conn_inf.sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (conn_inf.sockfd == -1)
      continue;

    if (connect(conn_inf.sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break; /* Success */

    close(conn_inf.sockfd);
  }

  if (rp == NULL) { /* No address succeeded */
    fputs("Could not connect\n", stderr);
    return -1;
  }

  freeaddrinfo(result); /* No longer needed */

  for (;;) {
    char buffer[max_msg_size];
    *buffer = '\0';
    get_user_input(buffer, max_msg_size, "Enter a string:\n");
    socklen_t len = strlen(buffer);

    if (write(conn_inf.sockfd, buffer, len) != len) {
      fputs("partial/failed write\n", stderr);
      return -1;
    }

    nread = read(conn_inf.sockfd, buf, max_msg_size);
    if (nread == -1) {
      perror("read");
      return -1;
    }

    printf("Received %ld bytes: %s\n", (long)nread, buf);
  }

  return 0;
}
