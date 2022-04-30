/******************************************************
 *   Funcionalidad para el set-up y comportamiento
 *   del servidor
******************************************************/
#include "../include/server_utils.h"

int init_server(int port, int backlog)
{
	int listenfd, yes=1;
	struct sockaddr_in address;
    struct sigaction sa;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Primero nos aseguramos de que nadie mas esta usando el puerto 
        Asi evitamos el error de "Address already in use" del S.O */
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    /* Si todo va bien, hacemos el bind() y el listen() */
    Bind(listenfd, (struct sockaddr *) &address, sizeof(address));

    Listen(listenfd, backlog);
    
    /* Handler para SIGCHLD */
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        LOG_ERR("No se pudo asignar handler a SIGINT");
        exit(1);
    }

    LOG_INFO("Servidor en escucha");
    return listenfd;
}

void sigchld_handler(int s)
{
    int saved_errno = errno;
    pid_t pid;
    int pstat;

    while( (pid = waitpid(-1, &pstat, WNOHANG)) > 0);

    errno = saved_errno;
    return;
}

void launch_service(int cli_fd)
{
    int pid;
    Http_request *request;

    pid = fork();
    if (pid < 0)            // error
        exit(EXIT_FAILURE);
    if (pid != 0)           // father
        return;

    // child process
    pid = getpid();
    LOG_INFO("Nuevo servicio [%d]", pid);

    /* Mapear peticion http a una estructura Http_request */
    request = httprequest_parse_and_map(cli_fd);

    /* Mostrar informacion de la request en la salida debug */
    if (request)
        httprequest_print(request);

    /* Evaluar la peticion en el modulo de respuesta */
    if (request)
        http_response_eval_request(request, cli_fd);

    close(cli_fd);
    LOG_INFO("Cerrando servicio [%d]", pid);
    httprequest_free(request);
    exit(EXIT_SUCCESS);
}

void wait_finished_services()
{
    int status = 0;
    pid_t wpid;
    
    while ((wpid = wait(&status)) > 0);
}

void accept_connection(int sockfd)
{
    struct sockaddr connection;
    int cli_fd;
    int len = sizeof(connection);

    if ( (cli_fd = accept(sockfd, (struct sockaddr*) &connection, &len)) < 0) {
        /* Si se manda una interrupcion con el servidor en escucha, no salir */
        if (errno == EINTR)
            return;
        else {
            perror("accept");
            LOG_ERR("Error accepting connection");
            exit(EXIT_FAILURE);
        }
    }

    launch_service(cli_fd);
    close(cli_fd);
    wait_finished_services();

    return;
}

char *read_file(const char *filename)
{
    char *extension = NULL;

    if(!filename) return NULL;

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        LOG_ERR("No se pudo abrir fichero %s", filename);
        return NULL;
    }
    
    char *data = calloc(1, sizeof(char));
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        data = realloc(data, (strlen(data) + strlen(line) + 1) * sizeof(char));
        strcat(data, line);
    }

    fclose(fp);

    if (line)
        free(line);

    return data;
}
