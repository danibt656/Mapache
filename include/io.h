#ifndef _IO_H
#define _IO_H

#include "liblog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file(const char *filename);

char *read_file_from_FILE(FILE *fp);

#endif // _IO_H