#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "libsocket.h"
#include "httplib.h"
#include "map_parser.h"
#include "liblog.h"
#include "io.h"

#include <arpa/inet.h>
#include <pthread.h>

#define THREAD_POOL_SIZE 10

int init_server(int port, int backlog);

void sigchld_handler(int s);

void* thread_function(void* arg);
void* handle_request(void *p_client_socket);

void wait_finished_services();

void accept_connection(int sockfd);

#endif
