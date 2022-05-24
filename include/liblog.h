#ifndef _LIBLOG_H
#define _LIBLOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define USE_COLORS 0
#define NOT_USE_COLORS 1

typedef enum {
    LOG_T_INFO,
    LOG_T_ERR
} log_t;

/**
 * Init logger variables
*/
void set_logger(FILE *fout, int use_colors);

FILE* log_out_file;
int log_use_colors;
#define LOG_INFO(...) _log(LOG_T_INFO, log_out_file, "-> ", "\n", __VA_ARGS__)
#define LOG_ERR(...) _log(LOG_T_ERR, log_out_file, "!> ", "\n", __VA_ARGS__)

/**
 * Color functions
 *
 * @param fout Output file (stdout/external file)
*/
void liblog_red(FILE *fout);
void liblog_green(FILE *fout);
void liblog_bold_yellow(FILE *fout);
void liblog_reset_color(FILE *fout);

/**
 * Show timestamp in HH:MM:SS format
 *
 * @param fout Output file
*/
void liblog_timestamp(FILE *fout);

void _log(log_t type, FILE* fout, const char* prefix, const char* suffix, const char* fmt, ...);

#endif // _LIBLOG_H
