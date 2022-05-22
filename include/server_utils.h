#ifndef _SERVER_UTILS_H
#define _SERVER_UTILS_H

#include "libsocket.h"
#include "httplib.h"
#include "map_parser.h"
#include "liblog.h"
#include "io.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define THREAD_POOL_SIZE 10

int init_server(int port, int backlog);

void sigchld_handler(int s);

void* thread_function(void* arg);
void* handle_request(void *p_client_socket);

void wait_finished_services();

/**
 * Accept entering connection
 * 
 * @param sockfd    Server's listening socket
 * @param ctx       SSL context, or NULL if no SSL is enabled
*/
void accept_connection(int sockfd, SSL_CTX* ctx);

/* OpenSSL Configuration */
SSL_CTX* create_context();
void configure_context(SSL_CTX* ctx);

#endif // _SERVER_UTILS_H
