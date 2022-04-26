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

#include <errno.h>
#include <limits.h>             // PATH_MAX
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>              // BUFSIZ
#include <stdlib.h>             // exit()
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
static int
recv_file (struct conninfo *conninfo)
{
  char buff[BUFSIZ];
  char filename[PATH_MAX];
  char *filename_ptr = filename;
  _Bool have_filename = 0;
  ssize_t n_bytes_recvd = 0;
  FILE *fp = NULL;
  _Bool f_exists = 0;
  size_t n_bytes_total = 0;
  bzero (buff, sizeof buff);

  struct pollfd pfds[1]; // More if you want to monitor more
  pfds[0].fd = conninfo->connfd;
  pfds[0].events = POLLIN; // Alert me when I can read() data from this socket without blocking.

  for (;;)
  {
    int num_events = poll(pfds, 1, 250); // 250 milliseconds
    if (num_events == 0)
    {
      puts ("Completed receiving data");
      break;
    }
    else
    {
      if (pfds[0].revents & POLLIN)
      {
        n_bytes_recvd = recv (conninfo->connfd, buff, sizeof (buff), 0);
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
              f_exists = 1;
              break;
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
    }
  }

  if (fp != NULL)
  {
    if (fclose (fp) == EOF)
      perror ("fclose() failed");
  }
  else
    puts("Error: No file was opened");

  if (n_bytes_recvd == -1)
  {
    perror ("recv() failed");
    exit (n_bytes_recvd);
  }

  pfds[0].events = POLLOUT; // Alert me when I can send() data to this socket without blocking.

  int num_events = poll(pfds, 1, 2000);

  if (num_events == -1) {
    perror("poll");
    exit(1);
  }

  if (num_events == 0) {
    printf("Poll timed out!\n");
  }
  else
  {
    if (pfds[0].revents & POLLOUT)
    {
      snprintf (buff, sizeof buff, "%s %li bytes", f_exists == 0 ? "Received " : "File already exists. Received", n_bytes_total);
      puts (buff);
      puts ("Sending confirmation to client");
      ssize_t s_r = send (pfds[0].fd, buff, strlen (buff) + 1, 0);
      if (s_r >=0)
        printf ("%li bytes sent\n", s_r);
      else
        perror ("send");
    }
  }

  putchar ('\n');
  puts ("Completed.");

  return f_exists;
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
  // sockfd only needed if more connections are desired
  if (close (conninfo->sockfd))
    perror("close() failed");

  if (conninfo->connfd < 0)
  {
    perror ("accept");
    return conninfo->connfd;
  }

  puts ("Client connected");
  putchar ('\n');

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

  int f_exists = recv_file (&conninfo);

  if (close (conninfo.connfd))
    perror("close() failed");

  return f_exists;
}
