#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Config file name */
#define CONFIG_FILE "mapache.conf"
/* Environment variables for server signature & root */
#define SIGNATURE_ENV "__MAP__SIGNATURE_ENV"
#define ROOT_ENV "__MAP__ROOT_ENV"
#define ROOT_SHORT "__MAP__ROOT_SHORT"
#define IP_ENV "__MAP__IP_ENV"
#define TLS_EN_ENV "__MAP__TLS_EN_ENV"
#define KEY_PEM_ENV "__MAP__KEY_PEM_ENV"
#define CERT_PEM_ENV "__MAP__CERT_PEM_ENV"

#define STRCMP(str1, str2) (strcmp(str1, str2) == 0)

#define strcpy(dest, src) strncpy(dest, src, strlen(src))

#endif // _UTILS_H