/*
 tcp_file_transfer_server.c
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

struct conninfo
{
  int port;
  int sockfd;
  int connfd;
};

/*
 * recv_file
 * receive data from the client, obtain the filename and write the
 * remaining data to a file
 *
 */
static void
recv_file (const int connfd)
{
  char buff[BUFSIZ];
  char filename[PATH_MAX];
  char *filename_ptr = filename;
  _Bool have_filename = 0;
  ssize_t n_bytes_recvd = 0;
  FILE *fp = NULL;
  size_t n_bytes_total = 0;

  while ((n_bytes_recvd = recv (connfd, buff, sizeof (buff), 0)) != 0)
  {
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
        if (access (filename, F_OK) == 0)
        {
          puts ("File already exists");
          exit (EXIT_FAILURE);
        }

        fp = fopen (filename, "wb");
        if (fp == NULL)
        {
          perror ("fopen:");
          exit (errno);
        }
        printf ("Receiving '%s'\n", filename);
      }

      if (fwrite (buf_file_dat_ptr, 1, n_bytes_recvd, fp) !=
          (size_t) n_bytes_recvd)
      {
        fputs ("Failed to write buff", stderr);
        exit (-1);
      }
      n_bytes_total += n_bytes_recvd;
    }
    printf ("bytes received: %li\r", n_bytes_total);
  }

  if (fclose (fp) == EOF)
    strerror (errno);

  puts ("\nCompleted.");

  return;
}

static int
accept_connection (struct conninfo *conninfo)
{
  struct sockaddr_in servaddr, cli;

  conninfo->sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (conninfo->sockfd == -1)
  {
    perror ("socket");
    return -1;
  }
  else
    puts ("Socket successfully created");
  bzero (&servaddr, sizeof (servaddr));

  servaddr.sin_family = AF_UNSPEC;
  servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  servaddr.sin_port = htons (conninfo->port);

  // Binding newly created socket to given IP and verification
  if ((bind
       (conninfo->sockfd, (struct sockaddr *) &servaddr,
        sizeof (servaddr))) != 0)
  {
    perror ("bind");
    close (conninfo->sockfd);
    return -1;
  }
  else
    printf ("Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen (conninfo->sockfd, 5)) != 0)
  {
    perror ("listen");
    close (conninfo->sockfd);
    return -1;
  }
  else
    printf ("Server listening on port %d...\n", conninfo->port);

  socklen_t len = sizeof (cli);

  // Accept the data packet from client and verification
  conninfo->connfd =
    accept (conninfo->sockfd, (struct sockaddr *) &cli, &len);
  if (conninfo->connfd < 0)
  {
    perror ("accept");
    close (conninfo->sockfd);
    return conninfo->connfd;
  }
  else
    puts ("Client connected\n");

  return 0;
}

static void
show_usage (const char *prgname)
{
  printf ("Usage: %s [OPTIONS]\n\n", prgname);
  puts ("\
  -p <port>\n");

  return;
}

int
main (int argc, char *argv[])
{
  struct conninfo conninfo;
  int opt;
  int default_port = 8080;
  conninfo.port = default_port;

  while ((opt = getopt (argc, argv, "p:h")) != -1)
  {
    switch (opt)
    {
    case 'p':
      conninfo.port = atoi (optarg);
      break;
    case 'h':
    default:
      show_usage (argv[0]);
      return 0;
    }
  }

  if (accept_connection (&conninfo) < 0)
    return -1;

  recv_file (conninfo.connfd);
  puts ("Closing socket");
  return close (conninfo.sockfd);
}
