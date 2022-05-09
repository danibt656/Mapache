#include "../include/cgi.h"


void exec_script(int cli_fd, char* path, char* args, char* ext, char* to_fill_content, long* size)
{
    /* Ejecucion del script */
    char exec[SCRIPT_CMD_SIZE];
    FILE* res_exec;
    char* content_aux = NULL;
    if(STRCMP(ext, "py")){
        if(!STRCMP(args, ""))
            sprintf(exec, "python3 %s %s", path, args);
        else
            sprintf(exec, "python3 %s", path);
    }else if(STRCMP(ext, "php")){
        if(!STRCMP(args, ""))
            sprintf(exec, "php %s %s", path, args);
        else
            sprintf(exec, "php %s", path);
    }

    LOG_INFO("Executing script: %s", exec);

    res_exec = popen(exec, "r");
    if (!res_exec) {
        LOG_ERR("Couldn't execute script %s", path);
        Http_response *error = http_response_get_error_response(ERR_500);
        httpresponse_send_error(error, cli_fd);
        httpresponse_free(error);
    }

    /* Read content + Content-Lenght */
    content_aux = read_file_from_FILE(res_exec);
    strcpy(to_fill_content, content_aux);
    long content_len_int = strlen(to_fill_content);

    *size += content_len_int;

    if(content_aux) free(content_aux);
}
