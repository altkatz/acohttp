#include <errno.h>
#include <sys/epoll.h>
 #include <sys/sendfile.h>
#include <unistd.h>
#include "co_io.h"
#include "http.h"
static void re_add_poll(int, int, int, void*);

int co_read(int fd, char *buf, int count){
  int n;
  for(;;){
    n = read(fd, buf, count);
    if (-1 == n && EAGAIN == errno){
        acohttp_req* req = aco_get_arg();
        re_add_poll(req->epfd, fd, EPOLLIN, req);
        aco_yield();
        continue;
    }
    return n;
  }
}
int co_readline(int fd, char *buf, int bufsize){
  char *bufp = buf;
  while(1){
    int n = co_read(fd, bufp, bufsize);
    if(n <= 0){
      //error
      return n;
    } else {
      for (int i = 0; i < n ; i++){
        if (bufp - buf > bufsize){
          //overflow
          return -1;
        }
        if (*bufp == '\n'){
          return bufp-buf;
        }
        bufp++;
      }
    }
  }
}
int co_writen(int fd, char *buf, int count){
  char *bufp = buf;
  int left = count;
  while(left > 0){
    int n = write(fd, bufp, left);
    if (n <= 0){
      if (-1 == n && EAGAIN == errno){
          acohttp_req* req = aco_get_arg();
          re_add_poll(req->epfd, fd, EPOLLOUT, req);
          aco_yield();
          continue;
      } else {
        //error
        return n;
      }
    } else {
          bufp += n;
          left -= n;
    }
  }
  return count;
}
int co_writen_file(int fd, int filefd, int count){
  off_t offset = 0;
  while(offset < count){
    ssize_t n = sendfile(fd, filefd, &offset, count);
    if (n <= 0){
      if (-1 == n && EAGAIN == errno){
          acohttp_req* req = aco_get_arg();
          re_add_poll(req->epfd, fd, EPOLLOUT, req);
          aco_yield();
          continue;
      } else {
        //error
        return n;
      }
    } else {
      offset += n;
    }
  }
  return count;
}
static void re_add_poll(int epfd, int fd, int flag,void* ptr){
    struct epoll_event event;
    event.data.ptr = ptr;
    event.events = flag | EPOLLET | EPOLLONESHOT | EPOLLERR | EPOLLHUP;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}
