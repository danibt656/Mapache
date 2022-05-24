#ifndef _CFGPARSER_H
#define _CFGPARSER_H

#include "utils.h"
#include "liblog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEY_MAX_SIZE 128
#define VALUE_MAX_SIZE 512

#define SERVER_ROOT_KEY "server_root"
#define SERVER_SIGNATURE_KEY "server_signature"
#define SERVER_IP_KEY "server_ip"
#define LISTEN_PORT_KEY "listen_port"
#define MAX_CLIENTS_KEY "max_clients"
#define KEY_PEM_FILE "SSL_key"
#define CERT_PEM_FILE "SSL_cert"

typedef struct _CFG_PARSER_STRUCT {
    char* server_root;
    char* server_signature;
    char* server_ip;
    unsigned long int listen_port;
    unsigned long int max_clients;
    /* For HTTPS*/
    char* key_pem_file;
    char* cert_pem_file;
} cfg_parser;

cfg_parser* cfg_parser_init();

void cfg_parser_free(cfg_parser* parser);

static void __rm_wsp(char* s);

int parse_cfg_line(char* line, char* key, char* value);

int cfg_parser_parse(cfg_parser* parser, char* filename);

#endif // _CFGPARSER_H