#include <getopt.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netinet/in.h>
#include "libaco/aco.h"    
#include "util.h"
#include "http.h"
#define MAXEVENTS 1024


static const struct option long_options[]=
{
    {"help",no_argument,NULL,'?'},
    {"directory",required_argument,NULL,'d'},
    {"port",required_argument,NULL,'p'},
    {NULL,0,NULL,0}
};
static void usage() {
   fprintf(stderr,
	"acohttp [option]... \n"
	"  -d|--directory Specify root directory to serve. Default ./ .\n"
	"  -p|--port Specify listen port. Default to 3000 .\n"
	"  -?|-h|--help help.\n"
	);
}

int main(int argc, char* argv[]){
    int opt = 0;
    int options_index = 0;
    char *directory = "./";
    int port = 3000;

    while ((opt=getopt_long(argc, argv,"dp:?h",long_options,&options_index)) != EOF) {
        switch (opt) {
            case  0 : break;
            case 'd':
                directory = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case ':':
            case 'h':
            case '?':
                usage();
                return 0;
        }
    }
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
      fprintf(stderr,"Ignore SIGPIPE failed");
      return 0;
    }

    fprintf(stderr,"directory: %s port:%d\n",directory, port);
    int listenfd;
    struct sockaddr_in clientaddr;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));

    listenfd = open_listenfd(port);
    make_socket_non_blocking(listenfd);
    int epfd = epoll_create1(0);
    if (epfd <= 0){
      fprintf(stderr,"epoll_create1_err:%d\n",epfd);
      return -1;
    }

    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    acohttp_req listen_s;
    listen_s.fd = listenfd;
    event.data.ptr = (void*)(&listen_s);
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    socklen_t inlen = 1;
    aco_thread_init(NULL);
    aco_t* main_co = aco_create(NULL, NULL, 0, NULL, NULL);
    aco_share_stack_t* sstk = aco_share_stack_new(0);
    int timeout = 20;
    while (1) {
        int n = epoll_wait(epfd, events, MAXEVENTS, timeout);
        for (int i = 0; i < n; i++) {
            acohttp_req* r = (acohttp_req*)events[i].data.ptr;
            if (listenfd == r->fd) {
                int infd;
                while(1) {
                    infd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            break;
                        }
                    }

                    make_socket_non_blocking(infd);
                    fprintf(stderr, "new connection fd %d\n", infd);

                    acohttp_req *newreq = (acohttp_req *)malloc(sizeof(acohttp_req));
                    newreq->fd = infd;
                    newreq->co = aco_create(main_co, sstk, 0, http_handler, newreq);
                    event.data.ptr = newreq;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
                }   // end of while of accept

            } else {
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
		    close(r->fd);
                    fprintf(stderr,"epoll error fd: %d", r->fd);
                    continue;
                }

                fprintf(stderr,"new data from fd %d\n", r->fd);
                if (r->co->is_end != 1){
                  fprintf(stderr,"co not end\n");
                  aco_resume(r->co);
                } else {
                  fprintf(stderr,"co ended\n");
                }
            }
        }   //end of for
    }   // end of while(1)


}
