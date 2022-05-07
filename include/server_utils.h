#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "libsocket.h"
#include "httplib.h"
#include "map_parser.h"
#include "liblog.h"
#include "io.h"

int init_server(int port, int backlog);

void sigchld_handler(int s);

void launch_service(int connval);

void wait_finished_services();

void accept_connection(int sockfd);

#endif