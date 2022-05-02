#include "../include/daemonize.h"

void do_daemon(char* signature)
{
    pid_t pid = 0;

    pid = fork();
    if (pid < 0) {
        LOG_ERR("Error durante fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
        exit(EXIT_SUCCESS);     // exit father

    /* Child process */
    LOG_INFO("Demonizando servidor ...");

    /* Create new SessionID for child */
    if (setsid() < 0) {
        LOG_ERR("Error creando nuevo SessionID para proceso hijo");
        exit(EXIT_FAILURE);
    }

    /* Change Name of child */
    prctl(PR_SET_NAME, signature, NULL, NULL);

    /* Change mask so child is accessible to anybody */
    umask((mode_t) 0);

    /* Set daemon at root dir */
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