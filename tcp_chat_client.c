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

#define PORT 8080

void
func (int sockfd)
{
  char buff[BUFSIZ];
  int n;
  for (;;)
  {
    bzero (buff, sizeof (buff));
    fputs ("Enter the string : ", stdout);
    n = 0;
    while ((buff[n++] = getchar ()) != '\n');
    write (sockfd, buff, sizeof (buff));
    bzero (buff, sizeof (buff));
    read (sockfd, buff, sizeof (buff));
    printf ("From Server : %s", buff);
    if ((strncmp (buff, "exit", 4)) == 0)
    {
      printf ("Client Exit...\n");
      break;
    }
  }
}

int
main ()
{
  int sockfd, connfd;
  struct sockaddr_in servaddr, cli;

  // socket create and verification
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    printf ("socket creation failed...\n");
    exit (0);
  }
  else
    printf ("Socket successfully created..\n");
  bzero (&servaddr, sizeof (servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr ("127.0.0.1");
  servaddr.sin_port = htons (PORT);

  // connect the client socket to server socket
  if (connect (sockfd, (struct sockaddr *) & servaddr, sizeof (servaddr)) != 0)
  {
    printf ("connection with the server failed...\n");
    exit (0);
  }
  else
    printf ("connected to the server..\n");

  // function for chat
  func (sockfd);

  // close the socket
  return close (sockfd);
}