#include "http.h"
void http_handler(){
  acohttp_req* r = (acohttp_req *)aco_get_arg();
  printf("get_arg_in_co%d\n",r->fd);
  aco_yield();
  printf("resume_co%d\n", r->fd);
  aco_exit();
}
