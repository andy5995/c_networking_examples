/*
// Adapted from https://beej.us/guide/bgnet/html/#poll by Brian “Beej Jorgensen” Hall

 tcp_multiple_connections.c
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
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include "netex.h"

// Get sockaddr, IPv4 or IPv6:
void *
get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *) sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

// Add a new file descriptor to the set
void
add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
  // If we don't have room, add more space in the pfds array
  if (*fd_count == *fd_size)
  {
    *fd_size *= 2;              // Double it

    *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    if (pfds == NULL)
    {
      perror("malloc");
      exit(errno);
    }
  }

  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN;   // Check ready-to-read

  (*fd_count)++;
}

// Remove an index from the set
void
del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
  // Copy the one from the end over this one
  pfds[i] = pfds[*fd_count - 1];

  (*fd_count)--;
}

int
main(int argc, char *argv[])
{
  parse_server_opts(argc, argv);

  struct sockaddr_storage remoteaddr;   // Client address
  socklen_t addrlen;

  char buf[256];                // Buffer for client data

  char remoteIP[INET6_ADDRSTRLEN];

  // Start off with room for 5 connections
  // (We'll realloc as necessary)
  int fd_count = 0;
  int fd_size = 5;
  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
  if (pfds == NULL)
  {
    perror("malloc");
    return errno;
  }

  if (get_tcp_server_sockfd() < 0)
  {
    fputs("Error\n", stderr);
    free (pfds); // free the memory (and eliminate the -fanalyzer warning)
    return -1;
  }

  // Add the listener to set
  pfds[0].fd = conn_inf.sockfd;
  pfds[0].events = POLLIN;      // Report ready to read on incoming connection

  fd_count = 1;                 // For the listener

  // Main loop
  for (;;)
  {
    int poll_count = poll(pfds, fd_count, -1);

    if (poll_count == -1)
    {
      perror("poll");
      break;
    }

    // Run through the existing connections looking for data to read
    for (int i = 0; i < fd_count; i++)
    {

      // Check if someone's ready to read
      if (pfds[i].revents & POLLIN)
      {                         // We got one!!

        if (pfds[i].fd == conn_inf.sockfd)
        {
          // If listener is ready to read, handle new connection
          addrlen = sizeof remoteaddr;

          // Newly accept()ed socket descriptor
          int newfd =
            accept(conn_inf.sockfd, (struct sockaddr *) &remoteaddr,
                   &addrlen);

          if (newfd == -1)
          {
            perror("accept");
          }
          else
          {
            add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

            printf("pollserver: new connection from %s on "
                   "socket %d\n",
                   inet_ntop(remoteaddr.ss_family,
                             get_in_addr((struct sockaddr *) &remoteaddr),
                             remoteIP, INET6_ADDRSTRLEN), newfd);
          }
        }
        else
        {
          // If not the listener, we're just a regular client
          int sender_fd = pfds[i].fd;
          int nbytes = recv(sender_fd, buf, sizeof buf, 0);

          if (nbytes <= 0)
          {
            // Got error or connection closed by client
            if (nbytes == 0)
            {
              // Connection closed
              printf("pollserver: socket %d hung up\n", sender_fd);
            }
            else
            {
              perror("recv");
            }

            close(pfds[i].fd);  // Bye!

            del_from_pfds(pfds, i, &fd_count);

          }
          else
          {
            // We got some good data from a client

            for (int j = 0; j < fd_count; j++)
            {
              // Send to everyone!
              int dest_fd = pfds[j].fd;

              // Except the listener and ourselves
              // Except the listener and ourselves
              if (dest_fd != conn_inf.sockfd && dest_fd != sender_fd)
              {
                if (send(dest_fd, buf, nbytes, 0) == -1)
                {
                  perror("send");
                }
              }
            }
          }
        }                       // END handle data from client
      }                         // END got ready-to-read from poll()
    }                           // END looping through file descriptors
  }
                               // END for(;;)--and you thought it would never end!
  free(pfds);
  return errno;
}
