#ifndef CGI_H
#define CGI_H

#include "liblog.h"
#include "httplib.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * Execute a script.
 *
 * Must be given:
 *  - path  -> Script path without arguments
 *  - ext   -> Script extension (lang)
 *  - args  -> Script arguments
 *
 * These arguments will be filled:
 *  - to_fill_content   -> Script execution result
 *  - size              -> Result size in bytes
 *
 * @param cli_fd            Client connection socket
 * @param path              Relative path of script
 * @param args              Script args
 * @param ext               Script file extension (lang)
 * @param to_fill_content   Script execution script
 * @param size              Result size in bytes
*/
void exec_script(int cli_fd, char* path, char* args, char* ext, char* to_fill_content, long* size);

#endif