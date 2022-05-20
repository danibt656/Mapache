#ifndef _LIBSOCKET_H
#define _LIBSOCKET_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "../include/liblog.h"

/* Config file name */
#define CONFIG_FILE "mapache.conf"
/* Environment variables for server signature & root */
#define SIGNATURE_ENV "__MAP__SIGNATURE_ENV"
#define ROOT_ENV "__MAP__ROOT_ENV"
#define ROOT_SHORT "__MAP__ROOT_SHORT"
#define IP_ENV "__MAP__IP_ENV"
/* Listen port */
#define PORT 8080
/* Data recv buffer length */
#define BUFFLEN 100
/* Default listen backlog size */
#define BACKLOG 3

/**
 *  POSIX socket library wrappers (error handling)
 * 
 *  Original idea: Unix Network Programming, Vol. 1 (W.R. Stevens)
 */
int Socket(int family, int type, int protocol);
int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void Listen(int sockfd, int backlog);


/**
 *  Ensure that data (from buffer or from file) is fully sent through
 *  a TCP connection
 *
 *  This is necessary because 1 single call to send() doesn't ensure the
 *  transmission of the whole data stream
 *
 *  @param fd     Socket file descriptor
 *  @param vptr   Buffer pointer
 *  @param filename
 *  @param len    Buffer size (bytes)
 *
 *  @return       Num of bytes sent, or -1 if error
 *
 *  Original idea: Unix Network Programming, Vol. 1 (W.R. Stevens)
*/
/* Enviar todo el contenido de buff con send() */
int sendall(int fd, char *vptr, int *len);
/* Enviar todo el contenido de un archivo */
void send_file(const char *filename, int sockfd);

#endif // _LIBSOCKET_H
