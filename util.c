#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

#ifdef _WIN32
#include <windows.h>
#endif

volatile sig_atomic_t stop = 0;

#ifdef _WIN32
BOOL WINAPI console_handler(DWORD signal_type) {
  switch (signal_type) {
  case CTRL_C_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    stop = 1;
    return TRUE;
  default:
    return FALSE;
  }
}
#else
void signal_handler(int signum) {
  if (signum == SIGINT) {
    stop = 1;
  }
}
#endif

static void show_server_usage(const char *prgname) {
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  printf("\
  -p <port> (Optional; Default: %s)\n\n",
         default_port);
  return;
}

static void show_client_usage(const char *prgname) {
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  printf("\
  -a <host>\n\
  -p <port> (Optional; Default: %s)\n\n",
         default_port);
  return;
}

void parse_server_opts(const int argc, char *argv[], struct connection *conn_info) {
  conn_info->port = default_port;
  int opt;

  while ((opt = getopt(argc, argv, "p:h")) != -1) {
    switch (opt) {
    case 'p':
      conn_info->port = optarg;
      break;
    case 'h':
    default:
      show_server_usage(argv[0]);
      exit(0);
    }
  }

  return;
}

void parse_client_opts(const int argc, char *argv[], struct connection *conn_info) {
  conn_info->port = default_port;
  int opt;

  while ((opt = getopt(argc, argv, "a:p:h")) != -1) {
    switch (opt) {
    case 'p':
      conn_info->port = optarg;
      break;
    case 'a':
      conn_info->host = optarg;
      break;
    case 'h':
    default:
      show_client_usage(argv[0]);
      exit(EXIT_SUCCESS);
    }
  }

  if (!conn_info->host) {
    fputs("-a <host> is required\n", stderr);
    exit(EXIT_FAILURE);
  }

  return;
}

void get_user_input(char *buffer, size_t size, const char *prompt) {
  if (prompt) {
    printf("%s", prompt);
    fflush(stdout);
  }

  if (fgets(buffer, size, stdin)) {
    // Remove trailing newline if it exists
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
  } else {
    // fgets failed — clear buffer
    buffer[0] = '\0';
  }
}
