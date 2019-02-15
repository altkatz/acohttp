#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "http.h"
#include "co_io.h"
static int get_uri();
#define MY_RESP_HEADER "HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n"
#define MY_RESP_HEADER_404 "HTTP/1.0 404 NOT FOUND\nConnection: close\n\n"
#define MY_RESP_HEADER_403 "HTTP/1.0 403 FORBIDDEN\nConnection: close\n\n"
#define LINEBUFSIZE 1024
#define FILENAMEBUFSIZE 1024
static void cleanup();
extern char *rootdirectory;
void http_handler(){
  acohttp_req* r = (acohttp_req *)aco_get_arg();
  char buf[LINEBUFSIZE] = {0};
  int nc = co_readline(r->fd,buf,LINEBUFSIZE);
  if (nc <= 0){
    fprintf(stderr, "readline error %d\n", nc);
    cleanup();
  }
  char uri[LINEBUFSIZE] = {0};
  int uri_rc = get_uri(buf, sizeof(buf), uri, sizeof(uri));
  if(uri_rc != 0){
    cleanup();
  }
  printf("uri:%s\n",uri);
  char filename[FILENAMEBUFSIZE] = {0};
  strncpy(filename, rootdirectory, LINEBUFSIZE);
  strcat(filename, "/");
  strcat(filename, uri);
  printf("filename:%s\n",filename);
  struct stat sbuf;
  if(stat(filename, &sbuf) < 0) {
    fprintf(stderr,"404%s",filename);
    co_writen(r->fd,MY_RESP_HEADER_404,sizeof(MY_RESP_HEADER_404));
    cleanup();
  }

  if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
    fprintf(stderr,"403%s",filename);
    co_writen(r->fd,MY_RESP_HEADER_403,sizeof(MY_RESP_HEADER_403));
    cleanup();
  }
  int srcfd = open(filename, O_RDONLY, 0);
  if(srcfd < 2){
    fprintf(stderr,"404%s",filename);
    co_writen(r->fd,MY_RESP_HEADER_404,sizeof(MY_RESP_HEADER_404));
    cleanup();
  }
  co_writen(r->fd,MY_RESP_HEADER,sizeof(MY_RESP_HEADER));
  co_writen_file(r->fd, srcfd, sbuf.st_size);
  close(srcfd);
  cleanup();
}
static void cleanup(){
  acohttp_req* r = (acohttp_req *)aco_get_arg();
  close(r->fd);
  free(r);
  aco_exit();
}
static int get_uri(char *buf, int len, char *uri, int urilen){
  int last_slash = -1;
  for (int i = 0; i < len; i++){
    if(buf[i] == '/'){
      last_slash = i;
    } else if(buf[i] == ' '){
      if (last_slash != -1 ){
        strncpy(uri, buf+last_slash+1,i-last_slash-1);
        return 0;
      }
    }
  }
  return -1;
}
