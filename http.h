#pragma once
#include "libaco/aco.h"
typedef struct acohttp_req_t {
	int fd;
	aco_t* co;
} acohttp_req;
void http_handler();
