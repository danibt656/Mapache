#include "../include/server_utils.h"
#include "../include/queue.h"


pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_cond_var = PTHREAD_COND_INITIALIZER;

int init_server(int port, int backlog)
{
	int listenfd, yes=1;
	struct sockaddr_in address;
    struct sigaction sa;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
        /* Set IP accordingly to config file */
    char *server_ip = NULL;
    server_ip = getenv(IP_ENV);
    if (STRCMP(server_ip, "localhost"))
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        address.sin_addr.s_addr = inet_addr(server_ip);
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

    /* Thread pool */
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, &thread_function, NULL);
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

void* handle_request(void* p_client_socket)
{
    char* is_tls_enabled = NULL;
    is_tls_enabled = getenv(TLS_EN_ENV);

    LOG_INFO("New service [%lu]", pthread_self());

    Http_request* request = NULL;
    request = httprequest_parse_and_map(p_client_socket);

    if (request != NULL)
        http_response_eval_request(request, p_client_socket);

    httprequest_free(request);
    
    if (is_tls_enabled == NULL)
        close(*(int*)p_client_socket);
    else {
        SSL_shutdown((SSL*)p_client_socket);
        SSL_free((SSL*)p_client_socket);
    }
    free(p_client_socket);

    LOG_INFO("Ending service [%lu]", pthread_self());
}

void* thread_function(void* arg)
{
    while (1) {
        int* pclient;
        pthread_mutex_lock(&mutex);
        /* try to get a connection */
        if ((pclient = dequeue()) == NULL) {
            pthread_cond_wait(&thread_cond_var, &mutex);
            /* try again */
            pclient = dequeue();
        }
        pthread_mutex_unlock(&mutex);
        if (pclient != NULL) {
            handle_request(pclient);
        }
    }
}

void wait_finished_services()
{
    int status = 0;
    pid_t wpid;
    
    while ((wpid = wait(&status)) > 0);
}

void accept_connection(int sockfd, SSL_CTX* ctx)
{
    struct sockaddr connection;
    pthread_t thread;
    int cli_fd;
    SSL* ssl = NULL;
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
    
    if (ctx != NULL) {
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cli_fd);
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
    }

    int* pclient = malloc(sizeof(int));
    if (ssl != NULL)
        *pclient = *(int*)ssl;
    else
        *pclient = cli_fd;
    pthread_mutex_lock(&mutex);
    enqueue(pclient);
    pthread_cond_signal(&thread_cond_var);
    pthread_mutex_unlock(&mutex);

    wait_finished_services();

    return;
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        LOG_ERR("Unable to create SSL context");
        perror("SSL_CTX_new");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    if (!ctx) return;

    /* Get Key/Cert filenames */
    char* key_pem_file = getenv(KEY_PEM_ENV);
    if (key_pem_file == NULL)
        return;
    char* cert_pem_file = getenv(CERT_PEM_ENV);
    if (cert_pem_file == NULL)
        return;

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, cert_pem_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_pem_file, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
