#include "../include/liblog.h"

/*
    Funcion para inicializar variables globales de Logger
*/
void set_logger(FILE *fout, int use_colors)
{
    log_out_file = fout;
    log_use_colors = use_colors;
}

void liblog_red(FILE *fout) { fprintf(fout, "\033[1;31m"); }      // Rojo
void liblog_green(FILE *fout) { fprintf(fout, "\e[0;92m"); }      // Verde
void liblog_reset_color(FILE *fout) { fprintf(fout, "\033[0m"); } // Resetear
/**
 * Mostrar marca de tiempo en formato HH:MM:SS
 *
 * @param fout Fichero de salida
*/
void liblog_timestamp(FILE *fout)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(fout, "[%02d:%02d:%02d] ", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/**
 * Funcion generica que imprime texto por un fichero de salida
 * 
 * @param type     Tipo de log, para indicador (info->verde, error->rojo)
 * @param fout     Fichero de salida (normalmente stdout, stderr)
 * @param prefix   Informacion antes del texto
 * @param suffix   Caracteres despues del texto (normalmente \n)
 * @param fmt      Variables de formato (para los %d, %s, etc)
*/
void _log(log_t type, FILE* fout, const char* prefix, const char* suffix, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    liblog_timestamp(fout);
    if (prefix) {
        /* Poner color primero (rojo->error, verde->info) */
        if (log_use_colors == USE_COLORS)
            (type == LOG_T_INFO) ? liblog_green(fout) : liblog_red(fout);
        fputs(prefix, fout);

        if (log_use_colors == USE_COLORS)
            liblog_reset_color(fout);
    }
    /* Imprimir mensajes */
    vfprintf(fout, fmt, ap);
    if (suffix) fputs(suffix, fout);
    va_end(ap);
}