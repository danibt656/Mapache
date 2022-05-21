#include "../include/cfgparser.h"


cfg_parser* cfg_parser_init()
{
    cfg_parser* parser = malloc(sizeof(cfg_parser));

    parser->server_ip = NULL;
    parser->server_root = NULL;
    parser->server_signature = NULL;
    parser->listen_port = -1;
    parser->max_clients = -1;

    return parser;
}

void cfg_parser_free(cfg_parser* parser)
{
    if (parser->server_ip) free(parser->server_ip);
    if (parser->server_root) free(parser->server_root);
    if (parser->server_signature) free(parser->server_signature);
    free(parser);
}

void __rm_wsp(char* s) {
    char* d = s;
    do while(isspace(*s)) s++; while(*d++ = *s++);
}

int parse_cfg_line(char* line, char* key, char* value)
{
    char* tok = NULL;
    tok = strtok(line, "=");
    __rm_wsp(tok);
    sprintf(key, "%s", tok);
    tok = strtok(NULL, "\n");
    __rm_wsp(tok);
    sprintf(value, "%s", tok);
    return 3;
}

int cfg_parser_parse(cfg_parser* parser, char* filename)
{
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char key[KEY_MAX_SIZE] = "", value[VALUE_MAX_SIZE] = "";

    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    while((read = getline(&line, &len, fp)) != -1) {
        /* Skip comments */
        if (line[0] == '#') continue;
        int tokens_read = parse_cfg_line(line, key, value);
        if (STRCMP(key, SERVER_IP_KEY)) {
            parser->server_ip = malloc(strlen(value)+1);
            if (parser->server_ip == NULL)
                return -1;
            strcpy(parser->server_ip, value);
        }
        else if (STRCMP(key, SERVER_ROOT_KEY)) {
            parser->server_root = malloc(strlen(value)+1);
            if (parser->server_root == NULL)
                return -1;
            strcpy(parser->server_root, value);
        }
        else if (STRCMP(key, SERVER_SIGNATURE_KEY)) {
            parser->server_signature = malloc(strlen(value)+1);
            if (parser->server_signature == NULL)
                return -1;
            strcpy(parser->server_signature, value);
        }
        else if (STRCMP(key, LISTEN_PORT_KEY)) {
            parser->listen_port = (unsigned long int)atoi(value);
        }
        else if (STRCMP(key, MAX_CLIENTS_KEY)) {
            parser->max_clients = (unsigned long int)atoi(value);
        }
    }

    fclose(fp);
    if (line)
        free(line);
    return 0;
}