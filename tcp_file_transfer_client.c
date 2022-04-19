/*
 tcp_file_transfer_client.c
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
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void
func (const int sockfd, const char *file)
{
  FILE *fp = fopen (file, "rb");
  if (fp == NULL)
  {
    strerror (errno);
    exit (errno);
  }
  int r = fseek (fp, 0, SEEK_END);
  if (r != 0)
  {
    strerror (errno);
    exit (errno);
  }
  long len = ftell (fp);
  printf ("file length: %li\n", len);
  rewind (fp);

  send (sockfd, file, strlen (file) + 1, 0);
  char buff[BUFSIZ];

  size_t n_bytes_total = 0;
  do
  {
    size_t num;                 // = MIN (len, sizeof (buff));
    num = fread (buff, 1, sizeof (buff), fp);
    if (ferror (fp) != 0)
    {
      fputs ("error: fread", stderr);
      exit (-1);
    }
    send (sockfd, buff, num, 0);
    n_bytes_total += num;
    printf ("bytes sent: %li\n", n_bytes_total);
  }
  while (feof (fp) == 0);

  printf ("Sent %li bytes\n", n_bytes_total);

  bzero (buff, sizeof (buff));
  // read (sockfd, buff, sizeof (buff));
  printf ("From Server : %s", buff);
  if ((strncmp (buff, "exit", 4)) == 0)
  {
    printf ("Client Exit...\n");
  }
  if (fclose (fp) == EOF)
  {
    strerror (errno);
  }
}

static void
show_usage (const char *prgname)
{
  printf ("Usage: %s [OPTIONS]\n\n", prgname);
  puts ("\
  -a <address>\n\
  -p <port>\n\
  -f <file>\n");
  return;
}

int
main (int argc, char *argv[])
{
  int opt;
  char *default_host = "127.0.0.1";
  char *host = default_host;
  const int default_port = 8080;
  int port = default_port;
  char *file = NULL;

  while ((opt = getopt (argc, argv, "f:a:p:h")) != -1)
  {
    switch (opt)
    {
    case 'f':
      file = optarg;
      break;
    case 'p':
      port = atoi (optarg);
      break;
    case 'a':
      host = optarg;
      break;
    case 'h': default:
      show_usage (argv[0]);
      return 0;
    }
  }

  if (file == NULL)
  {
    fputs ("A file must be specified (-f <filename>)\n", stderr);
    exit (EXIT_FAILURE);
  }

  int sockfd;
  struct sockaddr_in servaddr;

  // socket create and verification
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    printf ("socket creation failed...\n");
    return 0;
  }
  else
    printf ("Socket successfully created..\n");
  bzero (&servaddr, sizeof (servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr (host);
  servaddr.sin_port = htons (port);

  // connect the client socket to server socket
  if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) != 0)
  {
    printf ("connection with the server failed...\n");
    return 0;
  }
  else
    printf ("connected to the server..\n");

  // function for chat
  func (sockfd, file);

  // close the socket
  close (sockfd);
}
