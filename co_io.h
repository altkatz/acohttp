#pragma once
#include "libaco/aco.h"
int co_read(int fd, char *buf, int count);
int co_readline(int fd, char *buf, int bufsize);
int co_writen(int fd, char *buf, int n);
int co_writen_file(int fd, int filefd, int n);
