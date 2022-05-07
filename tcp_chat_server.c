/*
// Adapted from https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

 tcp_chat_server.c
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
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "netex.h"

// Function designed for chat between client and server.
void
func()
{
  char buff[BUFSIZ];
  int n;
  // infinite loop for chat
  for (;;)
  {
    bzero(buff, BUFSIZ);

    // read the message from client and copy it in buffer
    read(conn_inf.connfd, buff, sizeof(buff));
    // print buffer which contains the client contents
    printf("From client: %s\t To client : ", buff);
    bzero(buff, BUFSIZ);
    n = 0;
    // copy server message in the buffer
    while ((buff[n++] = getchar()) != '\n');

    // and send that buffer to client
    write(conn_inf.connfd, buff, sizeof(buff));

    // if msg contains "Exit" then server exit and chat ended.
    if (strncmp("exit", buff, 4) == 0)
    {
      printf("Server Exit...\n");
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

  while ((opt = getopt(argc, argv, "p:h")) != -1)
  {
    switch (opt)
    {
    case 'p':
      conn_inf.port = optarg;
      break;
    case 'h':
    default:
      show_usage(argv[0]);
      return 0;
    }
  }

  if (get_tcp_server_sockfd() < 0)
  {
    fputs("Error\n", stderr);
    return -1;
  }

  struct sockaddr_in cli;
  socklen_t len = sizeof(cli);

  // Accept the data packet from client and verification
  conn_inf.connfd = accept(conn_inf.sockfd, (struct sockaddr *) &cli, &len);
  // sockfd only needed if more connections are desired
  if (close(conn_inf.sockfd))
    perror("close() failed");

  if (conn_inf.connfd < 0)
  {
    perror("accept");
    return conn_inf.connfd;
  }

  // Function for chatting between client and server
  func();

  return 0;
}
