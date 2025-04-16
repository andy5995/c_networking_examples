/*
// Adapted from
https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

 tcp_chat_client.c
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netex.h"

static void func(socket_t sockfd) {
  char buff[BUFSIZ];
  int n;
  for (;;) {
    memset(buff, 0, sizeof buff);
    fputs("Enter the string : ", stdout);
    n = 0;
    while ((buff[n++] = getchar()) != '\n')
      ;
    write(sockfd, buff, sizeof buff);
    memset(buff, 0, sizeof buff);
    read(sockfd, buff, sizeof buff);
    printf("From Server : %s", buff);
    if ((strncmp(buff, "exit", 4)) == 0) {
      printf("Client Exit...\n");
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  struct connection conn_info;
  parse_client_opts(argc, argv, &conn_info);

  assign_tcp_client_fd(&conn_info);

  // function for chat
  func(conn_info.sockfd);

  // close the socket
  close_socket_checked(conn_info.sockfd);

  return 0;
}
