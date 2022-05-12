/*
// Adapted from https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

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

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "netex.h"

void
func(int sockfd)
{
  char buff[BUFSIZ];
  int n;
  for (;;)
  {
    bzero(buff, sizeof(buff));
    fputs("Enter the string : ", stdout);
    n = 0;
    while ((buff[n++] = getchar()) != '\n');
    write(sockfd, buff, sizeof(buff));
    bzero(buff, sizeof(buff));
    read(sockfd, buff, sizeof(buff));
    printf("From Server : %s", buff);
    if ((strncmp(buff, "exit", 4)) == 0)
    {
      printf("Client Exit...\n");
      break;
    }
  }
}

static void
show_usage(const char *prgname)
{
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  puts("\
  -a <address>\n\
  -p <port>\n");
  return;
}

int
main(int argc, char *argv[])
{
  int opt;

  while ((opt = getopt(argc, argv, "a:p:h")) != -1)
  {
    switch (opt)
    {
    case 'p':
      conn_inf.port = optarg;
      break;
    case 'a':
      conn_inf.host = optarg;
      break;
    case 'h':
    default:
      show_usage(argv[0]);
      return 0;
    }
  }

  int res = get_tcp_client_sockfd();
  if (res < 0)
    return res;

  // function for chat
  func(conn_inf.sockfd);

  // close the socket
  return close(conn_inf.sockfd);
}
