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

    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    Bind(listenfd, (struct sockaddr *) &address, sizeof(address));

    Listen(listenfd, backlog);
    
    /* SIGCHLD Handler */
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        LOG_ERR("Couldn't assign SIGCHLD handler");
        exit(1);
    }

    LOG_INFO("Server listening");
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
    LOG_INFO("New service [%d]", pid);

    request = httprequest_parse_and_map(cli_fd);

    if (request)
        httprequest_print(request);

    if (request)
        http_response_eval_request(request, cli_fd);

    close(cli_fd);
    LOG_INFO("Closing service [%d]", pid);
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
        /* If interrupts are sent while listening, do not exit */
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
