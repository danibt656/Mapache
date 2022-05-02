#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/prctl.h>

#include "../include/liblog.h"

/**
 * Convertir el proceso maestro (actual) en demonio
*/
void do_daemon(char* signature);

#endif