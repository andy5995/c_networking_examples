/*
 example_tcp_file_transfer_server.c

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

#include <stdio.h>              // BUFSIZ
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <limits.h>             // PATH_MAX
#include <errno.h>
#include <stdlib.h>             // exit()
#include <libgen.h>             // basename()

// Function designed for file transfer between client and server.
void
func (const int connfd)
{
  char buff[BUFSIZ];
  char filename[PATH_MAX];
  char *base_filename = NULL;
  char *filename_ptr = filename;
  _Bool have_filename = 0;
  ssize_t n_bytes_recvd = 0;
  FILE *fp = NULL;
  size_t n_bytes_total = 0;

  do
  {
    bzero (buff, sizeof (buff));
    n_bytes_recvd = recv (connfd, buff, sizeof (buff), 0);
    if (n_bytes_recvd == 0)
      break;
    if (n_bytes_recvd == -1)
    {
      fputs ("error", stderr);
      exit (n_bytes_recvd);
    }

    char *buf_file_dat_ptr = buff;
    if (!have_filename)
    {
      ssize_t i;
      for (i = 0; i < n_bytes_recvd; i++)
      {
        *filename_ptr++ = buff[i];
        if (buff[i] == '\0')
        {
          have_filename = 1;
          // Skip to the byte after \0 (if there is any)
          i++;
          break;
        }
      }

      if (i == n_bytes_recvd)
        continue;

      buf_file_dat_ptr += i;
      n_bytes_recvd -= i;
    }

    if (have_filename)
    {
      if (fp == NULL)
      {
        base_filename = basename (filename);
        if (access (base_filename, F_OK) == 0)
        {
          puts ("File already exists");
          exit (EXIT_FAILURE);
        }

        fp = fopen (base_filename, "wb");
        if (fp == NULL)
        {
          perror ("fopen:");
          exit (errno);
        }
        puts ("opening file..");
      }

      if (fwrite (buf_file_dat_ptr, 1, n_bytes_recvd, fp) !=
          (size_t) n_bytes_recvd)
      {
        fputs ("Failed to write buff", stderr);
        exit (-1);
      }
      n_bytes_total += n_bytes_recvd;
    }
    printf ("bytes received: %li\n", n_bytes_total);
  }
  while (n_bytes_recvd != 0);
  if (fclose (fp) == EOF)
    strerror (errno);

  printf ("%li bytes written to '%s'\n", n_bytes_total, base_filename);

  return;
}

int
main (int argc, char *argv[])
{
  int opt;
  int default_port = 8080;
  int port = default_port;

  while ((opt = getopt (argc, argv, "p:")) != -1)
  {
    switch (opt)
    {
    case 'p':
      port = atoi (optarg);
      break;
    default:                   /* '?' */
      fprintf (stderr, "Usage: %s [-p <port>     (default is %d)\n", argv[0], default_port);
      exit (EXIT_FAILURE);
    }
  }

  struct sockaddr_in servaddr, cli;

  // socket create and verification
  int sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    printf ("socket creation failed...\n");
    return 0;
  }
  else
    printf ("Socket successfully created..\n");
  bzero (&servaddr, sizeof (servaddr));

  // assign IP, port
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  servaddr.sin_port = htons (port);

  // Binding newly created socket to given IP and verification
  if ((bind (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr))) != 0)
  {
    printf ("socket bind failed...\n");
    return 0;
  }
  else
    printf ("Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen (sockfd, 5)) != 0)
  {
    printf ("Listen failed...\n");
    return 0;
  }
  else
    printf ("Server listening on port %d...\n", port);

  socklen_t len = sizeof (cli);

  // Accept the data packet from client and verification
  int connfd = accept (sockfd, (struct sockaddr *) &cli, &len);
  if (connfd < 0)
  {
    printf ("server accept failed...\n");
    return 0;
  }
  else
    printf ("server accept the client...\n");

  // Function for chatting between client and server
  func (connfd);

  // After chatting close the socket
  close (sockfd);
}
