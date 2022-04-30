#include "../include/server_utils.h"
#include "../include/libsocket.h"
#include "../include/daemonize.h"

#include <confuse.h>
#include <getopt.h>
#include <signal.h>

#define ARG_FLAGS "dhf:"
#define SERVER_LOGO "misc/logo.txt"

#define ABS_ROOT_SIZE 256

/* Socket de escucha */
int server_fd;

/**
 *  Imprime ayuda sobre como usar el ejecutable
 */
void print_ayuda()
{   
    char* server_logo = read_file(SERVER_LOGO);
    if(server_logo) printf("%s\n", server_logo);
    free(server_logo);

    printf("USO: ./server <FLAGS>\n\n"
    "Las <FLAGS> son:\n"
    "   -h: Muestra la ayuda\n"
    "   -d: Demoniza el servidor\n"
    "   -f <FICHERO>: Redireccion del server log a <FICHERO>\n"
        );
}

/*
    Handler para signals encargado de cerrar descriptores
    utilizados por el servidor Mapache
*/
void mapache_handler(int sig)
{
    /* Cierra fichero de salida de logger (si se ha puesto flag -f) */
    if(log_out_file != stdout && log_out_file != stderr)
        fclose(log_out_file);
    /* Cierra descriptor de socket de escucha */
    if (server_fd)
        close(server_fd);
    
    /* Salir del programa */
    printf("[Ctrl+C] Mapache se despide\n");
    exit(EXIT_SUCCESS);
}

/**
 *  Rutina principal de servidor
 * 
 *  Se encarga de tomar la configuracion basica, y poner el servidor
 *  en escucha, listo para aceptar conexiones de clientes
 */
int main(int argc, char *argv[])
{
    int opt;
    int daemonize_enabled = 0;
    char* server_root = NULL, *server_signature = NULL;
    char server_abs_route[ABS_ROOT_SIZE];
    long int max_clients = BACKLOG, listen_port = PORT;

    /* Inicializar output de log */
    set_logger(stdout, USE_COLORS);

    /* Handler para Ctrl+C */
    sigset_t set, stopped;
    struct sigaction act;
    act.sa_handler = mapache_handler;
    act.sa_flags = 0;
    sigemptyset(&(act.sa_mask));

        /* Bloqueamos todas las senyales posibles */
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, &(act.sa_mask));
    
        /* Recoger SIGINT */
    if(sigaction(SIGINT, &act, NULL) < 0){
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Evaluar flags */
    while ((opt = getopt(argc, argv, ARG_FLAGS)) != -1) {
        switch (opt) {
            case 'd':
                daemonize_enabled = 1;
                LOG_INFO("Servidor demonio activado");
                break;
            case 'h':
                print_ayuda();
                exit(EXIT_SUCCESS);
            case 'f':
                log_out_file = fopen(optarg, "w");
                if(!log_out_file){
                    printf("No se ha podido abrir el fichero indicado\n");
                    exit(EXIT_FAILURE);
                }
                set_logger(log_out_file, NOT_USE_COLORS);
                break;
            case ':':
                LOG_ERR("La flag necesita un valor\n");
                exit(EXIT_FAILURE);
                break;
            case '?':
                LOG_ERR("Opcion desconocida: %c\n", optopt);
                print_ayuda();
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* Tomar configuracion de archivo .conf */
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

    /* Ajustar la signature a una variable de entorno */
    /* Asi podemos pasarla a otros modulos sin pasarla por parametro */
    if (!server_signature)
        server_signature = "Server";
    if (setenv(SIGNATURE_ENV, server_signature, 1) < 0) {
        perror("setenv");
        LOG_ERR("No se pudo poner 'SIGNATURE_ENV' a %s", server_signature);
        exit(EXIT_FAILURE);
    }
    /* Hacer lo propio con la server_root */
        /* Ruta absoluta del servidor */
    getcwd(server_abs_route, sizeof(server_abs_route));

    if (!server_root)
        server_root = "/web";
        /* Concatenar ruta absoluta del directorio actual con la raiz del servidor */
    strcat(server_abs_route, server_root);
        /* Poner raiz del servidor como variable de entorno */
    if (setenv(ROOT_ENV, server_abs_route, 1) < 0) {
        perror("setenv");
        LOG_ERR("No se pudo poner 'ROOT_ENV' a %s", server_root);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Iniciando servidor %s [Usar CTRL+C para detener]\n", server_signature);

    /* Poner socket en escucha */
    server_fd = init_server(listen_port, max_clients);

    /* Daemonizar proceso */
    if (daemonize_enabled) {
        do_daemon();
    }

    /* Aceptar conexiones */
    while (1)
        accept_connection(server_fd);

    if(log_out_file != stdout && log_out_file != stderr)
        fclose(log_out_file);
    if (server_fd)
        close(server_fd);
    return 0;
}