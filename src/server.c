#include "../include/server_utils.h"
#include "../include/libsocket.h"
#include "../include/daemonize.h"
#include "../include/io.h"

#include <confuse.h>
#include <getopt.h>
#include <signal.h>

#define ARG_FLAGS "dhf:"
#define SERVER_LOGO "misc/logo.txt"

#define ABS_ROOT_SIZE 256

#define DEFAULT_SIGNATURE "Mapache"


int server_fd;


void print_help()
{   
    char* server_logo = read_file(SERVER_LOGO);
    if(server_logo) printf("%s\n", server_logo);
    free(server_logo);

    printf("USAGE: ./server <FLAGS>\n\n"
    "Current <FLAGS> are:\n"
    "   -h: Shows this help\n"
    "   -d: Daemonizes server\n"
    "   -f <FILE>: Redirect server logs to <FILE>\n"
        );
}


void mapache_handler(int sig)
{
    /* Close output log file (if specified) */
    if(log_out_file != stdout && log_out_file != stderr)
        fclose(log_out_file);
    
    if (server_fd)
        close(server_fd);

    printf("[Ctrl+C] Closing server\n");
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
    int opt;
    int daemonize_enabled = 0;
    char* server_root = NULL, *server_signature = NULL;
    char server_abs_route[ABS_ROOT_SIZE];
    long int max_clients = BACKLOG, listen_port = PORT;

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
                LOG_INFO("Daemonizing is enabled");
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case 'f':
                log_out_file = fopen(optarg, "w");
                if(!log_out_file){
                    printf("Couldn't open specified file\n");
                    exit(EXIT_FAILURE);
                }
                set_logger(log_out_file, NOT_USE_COLORS);
                break;
            case ':':
                LOG_ERR("Flag needs an associated value\n");
                exit(EXIT_FAILURE);
                break;
            case '?':
                LOG_ERR("Unknown flag: %c\n", optopt);
                print_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* Get settings from .conf file */
    cfg_opt_t opts[] = {
        CFG_SIMPLE_STR("server_root", &server_root),
        CFG_SIMPLE_INT("max_clients", &max_clients),
        CFG_SIMPLE_INT("listen_port", &listen_port),
        CFG_SIMPLE_STR("server_signature", &server_signature),
        CFG_END()
    };
    cfg_t* cfg;
    cfg = cfg_init(opts, 0);
    cfg_parse(cfg, "server_conf.conf");
    if (!server_signature)
        server_signature = DEFAULT_SIGNATURE;
    if (setenv(SIGNATURE_ENV, server_signature, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set SIGNATURE_ENV as \'%s\'", server_signature);
        exit(EXIT_FAILURE);
    }
    getcwd(server_abs_route, sizeof(server_abs_route));
    if (!server_root)
        server_root = "/web";
        /* Concatenate absolute path to current directory with server root */
    strcat(server_abs_route, server_root);
        /* Set server root as environment variable */
    if (setenv(ROOT_ENV, server_abs_route, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set ROOT_ENV as \'%s\'", server_abs_route);
        exit(EXIT_FAILURE);
    }
    if (setenv(ROOT_SHORT, server_root, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set ROOT_ENV as \'%s\'", server_root);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Starting server...%s [Press CTRL+C to stop]\n", server_signature);

    server_fd = init_server(listen_port, max_clients);
    if (daemonize_enabled) {
        do_daemon(server_signature);
    }
    while (1)
        accept_connection(server_fd);

    if(log_out_file != stdout && log_out_file != stderr)
        fclose(log_out_file);
    if (server_fd)
        close(server_fd);
    return 0;
}
