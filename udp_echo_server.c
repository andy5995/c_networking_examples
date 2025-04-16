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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"

int main(int argc, char *argv[]) {
  struct connection conn_info;
  parse_server_opts(argc, argv, &conn_info);

  assign_udp_server_fd(&conn_info);

  /* Read datagrams and echo them back to sender */
  for (;;) {
    struct sockaddr_storage peer_addr;
    char buf[MAX_BUF_ECHO_MSG] = {0};
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
    ssize_t nread = recvfrom(conn_info.sockfd, buf, sizeof buf, 0, (struct sockaddr *)&peer_addr,
                             &peer_addr_len);

    if (nread == -1) {
      continue; /* Ignore failed request */
    }

    char host[NI_MAXHOST], service[NI_MAXSERV];
    int s = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host, NI_MAXHOST, service,
                        NI_MAXSERV, NI_NUMERICSERV);
    if (s == 0)
      printf("Received %ld bytes from %s:%s\n", (long)nread, host, service);
    else
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

    ssize_t r_sto =
        sendto(conn_info.sockfd, buf, nread, 0, (struct sockaddr *)&peer_addr, peer_addr_len);

    if (strncasecmp(buf, "exit", 4) == 0) {
      puts("Received 'exit'");
      close(conn_info.sockfd);
      return 0;
    }

    if (r_sto != nread) {
      fputs("Error sending response\n", stderr);
      close(conn_info.sockfd);
      return r_sto;
    }
  }
  return 0;
}
