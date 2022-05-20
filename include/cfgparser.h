#ifndef _CFGPARSER_H
#define _CFGPARSER_H

#include "liblog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_MAX_SIZE 128
#define VALUE_MAX_SIZE 512

#define SERVER_ROOT_KEY "server_root"
#define SERVER_SIGNATURE_KEY "server_signature"
#define SERVER_IP_KEY "server_ip"
#define LISTEN_PORT_KEY "listen_port"
#define MAX_CLIENTS_KEY "max_clients"

typedef struct _CFG_PARSER_STRUCT {
    char* server_root;
    char* server_signature;
    char* server_ip;
    unsigned long int listen_port;
    unsigned long int max_clients;
} cfg_parser;

cfg_parser* cfg_parser_init();

void cfg_parser_free(cfg_parser* parser);

int cfg_parser_parse(cfg_parser* parser, char* filename);

#endif // _CFGPARSER_H