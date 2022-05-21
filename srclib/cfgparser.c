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
    int toks_read = 0;

    /* Line format: key = value (whitespace is indiferent) */
    /* Key */
    tok = strtok(line, "=");
    if (tok == NULL) return -1;
    __rm_wsp(tok);
    sprintf(key, "%s", tok);
    toks_read++;
    /* Value */
    tok = strtok(NULL, "\n");
    if (tok == NULL) return -1;
    __rm_wsp(tok);
    sprintf(value, "%s", tok);
    toks_read++;
    return toks_read;
}

int cfg_parser_parse(cfg_parser* parser, char* filename)
{
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int linenum = 1;
    char key[KEY_MAX_SIZE] = "", value[VALUE_MAX_SIZE] = "";

    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    while((read = getline(&line, &len, fp)) != -1) {
        /* Skip comments */
        if (line[0] == '#') continue;
        /* Parse line */
        int tokens_read = parse_cfg_line(line, key, value);
        if (tokens_read < 0) {
            LOG_ERR("Syntax error in config file [line %d]", linenum);
            fclose(fp);
            if (line) free(line);
            return -1;
        }

        if (STRCMP(key, SERVER_IP_KEY)) {
            parser->server_ip = malloc(strlen(value)+1);
            if (parser->server_ip == NULL) {
                fclose(fp);
                if (line) free(line);
                return -1;
            }
            strcpy(parser->server_ip, value);
        }
        else if (STRCMP(key, SERVER_ROOT_KEY)) {
            parser->server_root = malloc(strlen(value)+1);
            if (parser->server_root == NULL) {
                fclose(fp);
                if (line) free(line);
                return -1;
            }
            strcpy(parser->server_root, value);
        }
        else if (STRCMP(key, SERVER_SIGNATURE_KEY)) {
            parser->server_signature = malloc(strlen(value)+1);
            if (parser->server_signature == NULL) {
                fclose(fp);
                if (line) free(line);
                return -1;
            }
            strcpy(parser->server_signature, value);
        }
        else if (STRCMP(key, LISTEN_PORT_KEY)) {
            parser->listen_port = (unsigned long int)atoi(value);
        }
        else if (STRCMP(key, MAX_CLIENTS_KEY)) {
            parser->max_clients = (unsigned long int)atoi(value);
        }

        linenum++;
    }

    fclose(fp);
    if (line) free(line);
    return 0;
}