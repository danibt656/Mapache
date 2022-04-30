#include "../include/daemonize.h"

/* Funciona para daemonizar el proceso actual */
void do_daemon()
{
    pid_t pid = 0;

    pid = fork();
    if (pid < 0) {
        LOG_ERR("Error durante fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
        exit(EXIT_SUCCESS);     // salir de padre

    /* Proceso hijo */
    LOG_INFO("Demonizando servidor ...");

    /* Crear nuevo SessionID para hijo */
    if (setsid() < 0) {
        LOG_ERR("Error creando nuevo SessionID para proceso hijo");
        exit(EXIT_FAILURE);
    }

    /*Cambiar la mascara del proceso para que sea accesible a cualquiera */
    umask((mode_t) 0);

    /* Establecer el directorio raiz para el demonio */
    if ( (chdir("/")) < 0) {
        LOG_ERR("Error cambiando el directorio: \"/\"");
        exit(EXIT_FAILURE);
    }

    LOG_INFO("Cerrando descriptores estandar");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return;
}