#pragma once
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "netex.h"

extern volatile sig_atomic_t stop;

#ifdef _WIN32_
BOOL WINAPI console_handler(DWORD signal_type);
#else
void signal_handler(int signum);
#endif

    void parse_server_opts(const int argc, char *argv[], struct socket_info_t *socket_info);
void parse_client_opts(const int argc, char *argv[], struct socket_info_t *socket_info);
void get_user_input(char *buffer, size_t size, const char *prompt);
