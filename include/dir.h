#ifndef DIR_H
#define DIR_H

#include "liblog.h"
#include "httplib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/**
 * @brief Determines wether a path is a directory or a file
 * 
 * @param path  The path to evaluate
 * 
 * @return 1 if it's a path, 0 if it's a file, -1 if error
 */
int path_is_directory(char *path);

/**
 * @brief Get the directory as index HTML response, filling up its contents
 * 
 * @param response  The HTTP response struct to fill
 * @param dirpath   The path to the directory
 */
char* get_directory_as_index(char *dirpath);

#endif