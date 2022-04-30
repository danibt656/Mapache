/*
 *  Funcionaliad para encapsular llamadas a la API
 *  de sockets
 *  
 *  @author Carlos Anivarro Batiste
 *  @author Daniel Barahona Martin
*/
#include "../include/libsocket.h"

/******************************************************
 *      Funciones Wrapper
 *  Encapsulan el control de errores
******************************************************/
int Socket(int family, int type, int protocol)
{
    int n;
    if ( (n = socket(family, type, protocol)) < 0) {
        perror("socket");
        LOG_ERR("Fallo al crear socket");
        exit(EXIT_FAILURE);
    }
    return n;
}

int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(sockfd, level, optname, optval, optlen) == -1) {
        perror("setsockopt");
        LOG_ERR("Fallo al obtener puerto para servidor");
        exit(EXIT_FAILURE);
    }   
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0) {
        perror("bind");
        LOG_ERR("Fallo al enlazar socket con direcction de host");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog)
{
    char *ptr;

    if (listen(sockfd, backlog) < 0) {
        perror("listen");
        LOG_ERR("Fallo al activar la escucha");
        exit(EXIT_FAILURE);
    }
}

/******************************************************
 *      Funciones de envio de datos
******************************************************/
int sendall(int fd, char *vptr, int *len)
{
    int total = 0, bytesleft = *len, n;

    while (total < *len) {
        n = send(fd, vptr+total, bytesleft, 0);
        if (n == -1)
            break;
        total += n;
        bytesleft -= n;
    }

    *len = total;       /* Retornar num bytes enviados aqui */

    /* -1 si fallo, 0 si exito */
    return n == -1 ? -1 : 0;
}

void send_file(const char *filename, int sockfd)
{
    int n;
    char data[BUFFLEN] = {0};
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        LOG_ERR("No se pudo leer fichero: %s", filename);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while(fread(data, 1, BUFFLEN+1, fp) != 0) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            perror("send");
            LOG_ERR("No se pudo enviar buffer desde fichero");
            fclose(fp);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        memset(&data, 0, BUFFLEN);
    }
    
    fclose(fp);
}
