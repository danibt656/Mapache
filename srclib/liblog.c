#include "../include/liblog.h"


void set_logger(FILE *fout, int use_colors)
{
    log_out_file = fout;
    log_use_colors = use_colors;
}

void liblog_red(FILE *fout) { fprintf(fout, "\033[1;31m"); }
void liblog_green(FILE *fout) { fprintf(fout, "\e[0;92m"); }
void liblog_bold_yellow(FILE *fout) { fprintf(fout, "\033[1;33m"); }
void liblog_reset_color(FILE *fout) { fprintf(fout, "\033[0m"); }

void liblog_timestamp(FILE *fout)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(fout, "(%02d:%02d:%02d) ", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/**
 * Generic log function
 * 
 * @param type     For the indicator (info->green, error->red)
 * @param fout     Output file (stdout, stderr)
 * @param prefix   Prefix to append before the text
 * @param suffix   Suffix to append after the text (normally \n)
 * @param fmt      Format data (%d, %s...)
*/
void _log(log_t type, FILE* fout, const char* prefix, const char* suffix, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    /* Signature */
    if (log_use_colors == USE_COLORS) {
        liblog_bold_yellow(fout);
    }
    char *signature = NULL;
    signature = getenv("SIGNATURE_ENV");
    if (signature == NULL)
        signature = "Mapache";
    fprintf(fout, "%s:", signature);

    /* Timestamp */
    liblog_timestamp(fout);
    if (log_use_colors == USE_COLORS) {
        liblog_reset_color(fout);
    }

    /* Prefix */
    if (prefix) {
        if (log_use_colors == USE_COLORS)
            (type == LOG_T_INFO) ? liblog_green(fout) : liblog_red(fout);
        fputs(prefix, fout);

        if (log_use_colors == USE_COLORS)
            liblog_reset_color(fout);
    }
    /* Message + format */
    vfprintf(fout, fmt, ap);

    /* Suffix */
    if (suffix)
        fputs(suffix, fout);
    va_end(ap);
}
