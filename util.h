#pragma once
#define LISTENQ     1024

int open_listenfd(int port);
int make_socket_non_blocking(int fd);
