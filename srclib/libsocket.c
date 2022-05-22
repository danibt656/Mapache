/*
 *  Funcionaliad para encapsular llamadas a la API
 *  de sockets
 *  
 *  @author Carlos Anivarro Batiste
 *  @author Daniel Barahona Martin
*/
#include "../include/libsocket.h"

/******************************************************
 *  Wrappers for POSIX-socket API
******************************************************/
int Socket(int family, int type, int protocol)
{
    int n;
    if ( (n = socket(family, type, protocol)) < 0) {
        perror("socket");
        LOG_ERR("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    return n;
}

int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(sockfd, level, optname, optval, optlen) == -1) {
        perror("setsockopt");
        LOG_ERR("Failed to get listening port");
        exit(EXIT_FAILURE);
    }   
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0) {
        perror("bind");
        LOG_ERR("Failed to bind server socket to host");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog)
{
    char *ptr;

    if (listen(sockfd, backlog) < 0) {
        perror("listen");
        LOG_ERR("Failed to start listening");
        exit(EXIT_FAILURE);
    }
}

/******************************************************
 *  Sending data
******************************************************/
ssize_t Send(void* fd, char* vptr, size_t len)
{
    ssize_t sent = 0;
    char* is_tls_enabled = NULL;
    is_tls_enabled = getenv(TLS_EN_ENV);
    if (is_tls_enabled == NULL)
        sent = send(*(int*)fd, vptr, len, 0);
    else
        sent = (ssize_t)SSL_write((SSL*)fd, vptr, len);
    return sent;
}

int sendall(void* fd, char *vptr, int *len)
{
    int total = 0, bytesleft = *len, n;

    while (total < *len) {
        n = Send(fd, vptr+total, bytesleft);
        if (n == -1)
            break;
        total += n;
        bytesleft -= n;
    }

    *len = total;   // Return number of bytes sent

    return n == -1 ? -1 : 0;
}

void send_file(const char *filename, void* sockfd)
{
    int n;
    char data[BUFFLEN] = {0};
    FILE *fp = fopen(filename, "r");

    ssize_t sent = 0;
    char* is_tls_enabled = NULL;
    is_tls_enabled = getenv(TLS_EN_ENV);

    if (fp == NULL) {
        LOG_ERR("Couldn't read file %s", filename);
        if (is_tls_enabled == NULL)
            close(*(int*)sockfd);
        else {
            SSL_shutdown((SSL*)sockfd);
            SSL_free((SSL*)sockfd);
        }
        exit(EXIT_FAILURE);
    }

    while(fread(data, 1, BUFFLEN+1, fp) != 0) {
        if (Send(sockfd, data, sizeof(data)) == -1) {
            perror("Send");
            LOG_ERR("Couldn't send buffer from file");
            fclose(fp);
            if (is_tls_enabled == NULL)
                close(*(int*)sockfd);
            else {
                SSL_shutdown((SSL*)sockfd);
                SSL_free((SSL*)sockfd);
            }
            exit(EXIT_FAILURE);
        }
        memset(&data, 0, BUFFLEN);
    }
    
    fclose(fp);
}
