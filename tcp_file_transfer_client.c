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
#include <libgen.h>             // basename()
#include <limits.h>
#include <poll.h>

int
func (int sockfd, const char *file)
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

  // basename() may modify the contents of 'file', so create a copy
  char file_orig[PATH_MAX];
  if ((size_t) snprintf (file_orig, sizeof file_orig, "%s", file) >=
      sizeof file_orig)
    fputs ("filename truncated", stderr);

  char *file_basename = basename (file_orig);
  printf ("Sending %s...\n", file);

  send (sockfd, file_basename, strlen (file_basename) + 1, 0);
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
    printf ("bytes sent: %li\r", n_bytes_total);

  }
  while (feof (fp) == 0);

  putchar ('\n');
  bzero (buff, sizeof (buff));
  fputs ("Server replied: ", stdout);
  int n_bytes_recvd;
  while ((n_bytes_recvd = recv (sockfd, buff, sizeof (buff), 0)) != 0)
  {
    fputs (buff, stdout);
    bzero (buff, sizeof buff);
  }

  if (n_bytes_recvd < 0)
    perror ("recv() failed");

  if (fclose (fp) == EOF)
  {
    perror ("fclose");
  }

  return strstr (buff, "already exists") != NULL;
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
  char *default_port = "8080";
  char *port = default_port;
  char *file = NULL;

  while ((opt = getopt (argc, argv, "f:a:p:h")) != -1)
  {
    switch (opt)
    {
    case 'f':
      file = optarg;
      break;
    case 'p':
      port = optarg;
      break;
    case 'a':
      host = optarg;
      break;
    case 'h':
    default:
      show_usage (argv[0]);
      return 0;
    }
  }

  if (file == NULL)
  {
    fputs ("A file must be specified (-f <filename>)\n", stderr);
    exit (EXIT_FAILURE);
  }

  struct addrinfo hints;
  struct addrinfo *result, *rp;

  /* Obtain address(es) matching host/port */
  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;        /* Any protocol */

  int s = getaddrinfo (host, port, &hints, &result);
  if (s != 0)
  {
    fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */
  int sockfd = -1;
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    sockfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1)
      continue;

    if (connect (sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
    {
      printf ("Connected to %s\n", host);
      break;
    }
    perror ("connect");
    if (close (sockfd) != 0)
      perror ("close");
    return -1;
  }

  freeaddrinfo (result);        /* No longer needed */

  if (sockfd == -1)
  {
    fputs ("Unable to create socket\n", stderr);
    return -1;
  }

  int f_exists = func (sockfd, file);

  puts ("\nClosing socket");
  if (close (sockfd) != 0)
    perror ("close() failed");

  return f_exists;
}
