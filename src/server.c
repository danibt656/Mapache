#include "../include/server_utils.h"
#include "../include/libsocket.h"
#include "../include/daemonize.h"
#include "../include/io.h"
#include "../include/cfgparser.h"

#include <getopt.h>
#include <signal.h>

#define ARG_FLAGS "dhf:C:s"
#define SERVER_LOGO "misc/logo.txt"

#define ABS_ROOT_SIZE 256

#define DEFAULT_SIGNATURE "Mapache"
#define DEFAULT_IP "localhost"
#define DEFAULT_ROOT "/web"


int server_fd;
extern pthread_t thread_pool[THREAD_POOL_SIZE];
SSL_CTX* ctx;

__attribute__((always_inline)) inline void print_help()
{   
    printf("USAGE: ./server <FLAGS>\n\n"
    "Current <FLAGS> are:\n"
    "   -h: Shows this help\n"
    "   -d: Daemonizes server\n"
    "   -s: Use OpenSSL-TLS to enable secure-HTTP (HTTPS) serving\n"
    "   -f <FILE>: Redirect server logs to <FILE>\n"
    "   -C <C_FILE>: Take configuration from <C_FILE> config file (default is `./%s`)\n",
        CONFIG_FILE
        );
}

void mapache_handler(int sig)
{
    /* Close output log file (if specified) */
    if(log_out_file != stdout && log_out_file != stderr)
        fclose(log_out_file);
    
    if (server_fd)
        close(server_fd);
    
    if (ctx)
        SSL_CTX_free(ctx);

    // for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    //     void* retval;
    //     pthread_join(thread_pool[i], &retval);
    // }
    // pthread_exit(NULL);
    printf("[Ctrl+C] Closing server\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int opt;
    int use_tls = 0;
    int daemonize_enabled = 0;
    char* cfg_filename = NULL;
    char server_abs_route[ABS_ROOT_SIZE];

    /* Init logger */
    set_logger(stdout, USE_COLORS);

    /* Ctrl+C handler */
    sigset_t set, stopped;
    struct sigaction act;
    act.sa_handler = mapache_handler;
    act.sa_flags = 0;
    sigemptyset(&(act.sa_mask));

        /* Block all signals possible */
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, &(act.sa_mask));
    
        /* Capture SIGINT (Ctrl+C) */
    if(sigaction(SIGINT, &act, NULL) < 0){
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Evaluate flags */
    while ((opt = getopt(argc, argv, ARG_FLAGS)) != -1) {
        switch (opt) {
            case 'd':
                daemonize_enabled = 1;
                LOG_INFO("[Config] Daemonizing is enabled");
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case 'f':
                log_out_file = fopen(optarg, "w");
                if(!log_out_file){
                    printf("[Config] Couldn't open specified file\n");
                    exit(EXIT_FAILURE);
                }
                set_logger(log_out_file, NOT_USE_COLORS);
                break;
            case 'C':
                cfg_filename = optarg;
                break;
            case 's':
                use_tls = 1;
                if (setenv(TLS_EN_ENV, "1", 1) < 0) {
                    perror("setenv");
                    LOG_ERR("Couldn't set TLS_EN_ENV as \'%c\'", '1');
                    exit(EXIT_FAILURE);
                }
                LOG_INFO("[Config] Secure-HTTP is enabled");
                break;
            case ':':
                LOG_ERR("[Config] Flag needs an associated value\n");
                exit(EXIT_FAILURE);
                break;
            case '?':
                LOG_ERR("[Config] Unknown flag: %c\n", optopt);
                print_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    cfg_parser* cfg;
    int parseret;
    cfg = cfg_parser_init();
    if (cfg_filename == NULL)
        parseret = cfg_parser_parse(cfg, CONFIG_FILE);
    else
        parseret = cfg_parser_parse(cfg, cfg_filename);
    if (parseret < 0) {
        LOG_ERR("Could not parse configuration file.");
        exit(EXIT_FAILURE);
    }

    if (!cfg->server_signature) {
        char* default_server_signature = DEFAULT_SIGNATURE;
        cfg->server_signature = malloc(strlen(default_server_signature)+1);
        strcpy(cfg->server_signature, default_server_signature);
    }
    if (setenv(SIGNATURE_ENV, cfg->server_signature, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set SIGNATURE_ENV as \'%s\'", cfg->server_signature);
        cfg_parser_free(cfg);
        exit(EXIT_FAILURE);
    }
    getcwd(server_abs_route, sizeof(server_abs_route));
    if (!cfg->server_root) {
        char* default_server_root = DEFAULT_ROOT;
        cfg->server_root = malloc(strlen(default_server_root)+1);
        strcpy(cfg->server_root, default_server_root);
    }
    /* Concatenate absolute path to current directory with server root */
    strcat(server_abs_route, cfg->server_root);
    /* Set server root as environment variable */
    if (setenv(ROOT_ENV, server_abs_route, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set ROOT_ENV as \'%s\'", server_abs_route);
        cfg_parser_free(cfg);
        exit(EXIT_FAILURE);
    }
    if (setenv(ROOT_SHORT, cfg->server_root, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set ROOT_ENV as \'%s\'", cfg->server_root);
        cfg_parser_free(cfg);
        exit(EXIT_FAILURE);
    }
    /* Check server IP & set environment variable */
    struct sockaddr_in sa;
    if (!cfg->server_ip || (inet_pton(AF_INET, cfg->server_ip, &(sa.sin_addr))) == 0) {
        if (cfg->server_ip) free(cfg->server_ip);
        char* default_server_ip = DEFAULT_IP;
        cfg->server_ip = malloc(strlen(default_server_ip)+1);
        strcpy(cfg->server_ip, default_server_ip);
    }
    if (setenv(IP_ENV, cfg->server_ip, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set IP_ENV as \'%s\'", cfg->server_ip);
        exit(EXIT_FAILURE);
    }
    /* Env.Vars for OpenSSL Key/Certificate filenames */
    if ((!cfg->key_pem_file || !cfg->cert_pem_file) && use_tls != 0) {
        LOG_ERR("No OpenSSL key-certficate pair was given!", cfg->server_signature);
        cfg_parser_free(cfg);
        exit(EXIT_FAILURE);
    }
    if (setenv(KEY_PEM_ENV, cfg->key_pem_file, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set KEY_PEM_ENV as \'%s\'", cfg->key_pem_file);
        cfg_parser_free(cfg);
        exit(EXIT_FAILURE);
    }
    if (setenv(CERT_PEM_ENV, cfg->cert_pem_file, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set CERT_PEM_ENV as \'%s\'", cfg->cert_pem_file);
        cfg_parser_free(cfg);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Starting server %s at %s port %ld... [Press CTRL+C to stop]\n",
            cfg->server_signature, cfg->server_ip, cfg->listen_port);

    server_fd = init_server(cfg->listen_port, cfg->max_clients);
    if (daemonize_enabled) {
        do_daemon(cfg->server_signature);
    }
    cfg_parser_free(cfg);

    /* TLS setp */
    if (use_tls != 0) {
        ctx = create_context();
        configure_context(ctx);
    } else
        ctx = NULL;

    while (1)
        accept_connection(server_fd, ctx);

    if(log_out_file != stdout && log_out_file != stderr)
        fclose(log_out_file);
    if (server_fd)
        close(server_fd);
    if (ctx)
        SSL_CTX_free(ctx);
    return 0;
}
