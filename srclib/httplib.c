#include "../include/httplib.h"
#include "../include/liblog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define AUX_SIZE 2048



// ****************************************************************************
//                                 HTTP Request
// ****************************************************************************
/* Estructura para una request */
struct _Http_request {
    char method[METHOD_SIZE];               // Metodo (GET, POST, OPTIONS)
    char path[PATH_SIZE];                   // Path (URL/URI)
    char post_args[POST_ARGS_SIZE];         // Argumentos (en caso de ser POST)
    struct phr_header headers[NUM_HEADERS]; // Cabeceras
    int num_headers;                        // Numero de cabeceras
    int version;                            // Version de HTTP
    int size;                               // Tamanio de request
};

Http_request* httprequest_init()
{
    Http_request* req = NULL;

    req = (Http_request*) calloc(1, sizeof(Http_request));
    if(!req) return NULL;

    strcpy(req->method, "");
    strcpy(req->path, "");
    strcpy(req->post_args, "");

    req->version = -1;
    req->size = -1;
    req->num_headers = 0;

    return req;
}

void httprequest_free(Http_request* req)
{
    if(req)
        free(req);
}

Http_request *httprequest_parse_and_map(int cli_fd)
{
    char buff[4096];
    const char *method, *path;
    int pret, minor_version, i;
    struct phr_header headers[100];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;
    Http_request* request;

    request = httprequest_init();
    if (!request) {
        return NULL;
    }

    while (1) {
        /* leer la request */
        while ((rret = read(cli_fd, buff + buflen, sizeof(buff) - buflen)) == -1 && errno == EINTR);

        if (rret <= 0)
            return NULL;
        prevbuflen = buflen;
        buflen += rret;

        /* parsear la request */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(buff, buflen, &method, &method_len, &path, &path_len,
                                &minor_version, headers, &num_headers, prevbuflen);
        
        if (pret > 0) {
            LOG_INFO("HTTP Request parseada");
            break;
        }
        
        else if (pret == -1) {
            return NULL;
        }

        /* request incompleta, continuar bucle */
        assert(pret == -2);

        if (buflen == sizeof(buff)) {
            return NULL;
        }
    }

    /* Introducir los datos en la estructura y validar */
    if (httprequest_set_all(request, buff, method_len,
                            method, path_len,path, minor_version,
                            pret, num_headers, headers) == HTTP_INVALID)
    {
        Http_response *error = http_response_get_error_response(ERR_400);
        httpresponse_send_error(error, cli_fd);
        httpresponse_free(error);
        return NULL;
    }

    return request;
}

int httprequest_set_all(Http_request* req,
                         char* buff,
                         int method_len,
                         const char* method,
                         int path_len,
                         const char* path,
                         int version,
                         int size,
                         int num_headers, 
                         struct phr_header *headers)
{
    char aux[AUX_SIZE];
    char *path_root = NULL;
    int i;

    /* Metodo */
    sprintf(aux, "%.*s", (int) method_len, method);
        /* Comprobacion de soporte */
    if (check_http_method(aux) == -1) {
        LOG_ERR("Metodo no soportado. Solo acepta GET, POST y OPTIONS.");
        return HTTP_INVALID;
    }
    strcpy(req->method, aux);
    memset(aux, 0, strlen(aux));

    /* Version */
        /* Comprobacion de soporte */
    if (version != HTTP_V_0 && version != HTTP_V_1) {
        LOG_ERR("Version HTTP no compatible. Solo acepta HTTP/1.0 o HTTP/1.1.");
        return HTTP_INVALID;
    }
    req->version = version;

    /* Path */
    char aux2[AUX_SIZE];
    sprintf(aux2, "%.*s", (int) path_len, path);
        /* Concatenar con server_root (fichero de configuracion) */
    path_root = getenv(ROOT_ENV);
    sprintf(aux, "%s%s", path_root, aux2);
    strcpy(req->path, aux);
    memset(aux, 0, strlen(aux));

    /* Size */
    req->size = size;

    /* Cabeceras */
    req->num_headers = num_headers;
    for (i = 0; i != num_headers; ++i) {
        req->headers[i].name_len = headers[i].name_len;
        req->headers[i].name = headers[i].name;
        req->headers[i].value_len = headers[i].value_len;
        req->headers[i].value = headers[i].value;
    }
    
    /* POST args */
    if (STRCMP(req->method, "POST")) {
        sprintf(aux, "%s", buff + req->size);
        strcpy(req->post_args, aux);
    }

    LOG_INFO("HTTP Request valida");
    return HTTP_VALID;
}

int check_http_method(char *method)
{
    if (STRCMP(method, "GET"))
        return 0;
    else if (STRCMP(method, "POST")) 
        return 0;
    else if (STRCMP(method, "OPTIONS"))
        return 0;
    else
        return -1;
}

void httprequest_print(Http_request* req)
{
    LOG_INFO("Tamanio: %d bytes", req->size);
    LOG_INFO("Comando: %s", req->method);
    LOG_INFO("Path: %s", req->path);
    LOG_INFO("Version HTTP: 1.%d", req->version);

    LOG_INFO("%d cabeceras:", req->num_headers);
    for (int i = 0; i < req->num_headers; i++) {
        LOG_INFO("%.*s: %.*s", (int)req->headers[i].name_len, (char*)req->headers[i].name, \
                                 (int)req->headers[i].value_len, (char*)req->headers[i].value);
    }
    if(strstr(req->method, "POST"))
        LOG_INFO("Argumentos del POST: %s\n", req->post_args);
}

// ****************************************************************************
//                                 HTTP RESPONSE
// ****************************************************************************
/* Estructura para una response */
struct _Http_response {
    int version;                            // Version de HTTP
    int code;                               // Codigo (200, 400, 404)
    char message[PATH_SIZE];                // Mensaje (OK, Not Found...)
    char headers[NUM_HEADERS][HEADER_SIZE]; // Cabeceras
    int num_headers;                        // Numero de cabeceras
    char *content;                          // Contenido de response
};

Http_response* httpresponse_init()
{
    Http_response* res = NULL;

    res = (Http_response*) calloc(1, sizeof(Http_response));
    if(!res) return NULL;

    res->version = HTTP_V_1;
    res->code = 200;
    strcpy(res->message, "OK");

    res->num_headers = NUM_HEADERS;
    for (int i = 0; i < NUM_HEADERS; i++)
        strcpy(res->headers[i], "");

    res->content = NULL;

    return res;
}

void httpresponse_free(Http_response* res)
{
    if (res->content)
        free(res->content);
    free(res);
}

void http_response_eval_request(Http_request *request, int cli_fd)
{
    Http_response *response = httpresponse_init(); 
    if(!response) return;

    /* Si es OPTIONS, responder */
    if (STRCMP(request->method, "OPTIONS")) {
        response->content = NULL;
        http_response_set_headers(response, NULL, NULL, -1);
        httpresponse_send_options(response, cli_fd);
        httpresponse_free(response);
        return;
    }

    /* Memoria para un conjunto de argumentos GET */
    char* args_for_get = malloc(strlen((request->path)+1)*sizeof(char*));
    if(!args_for_get){
        httpresponse_free(response);
        return;
    }

    /* Extraer los argumentos del path, si tiene */
    char *path;
    if(!(path = get_args_for_get(request->path, args_for_get))){
        free(args_for_get);
        httpresponse_free(response);
        return;
    }

    /* Mirar si el fichero pedido existe */
    if (access(path, F_OK) != 0) {
        LOG_ERR("404 Fichero %s no existe", path);
        httpresponse_free(response);
        free(args_for_get);
        /* Enviar error 404 not found */
        Http_response *error = http_response_get_error_response(ERR_404);
        httpresponse_send_error(error, cli_fd);
        httpresponse_free(error);
        return;
    }

    /* Get extension from request */
    char *ext = get_filename_extension(path);
    LOG_INFO("Extension: %s", ext);
    
    /* Enviar estructura en formato ASCII */
    char args_for_post[AUX_SIZE]="";
    if (STRCMP(request->method, "POST"))
        get_args_for_post(request->post_args, args_for_post);
    httpresponse_send_response(request, response, cli_fd, path, ext, args_for_get, args_for_post);
    

    LOG_INFO("HTTP Response enviada");

    /* Liberar memoria */
    free(args_for_get);
    httpresponse_free(response);
}

void http_response_set_headers(Http_response* response, char *path, char *ext, int is_script)
{
    /* Date header */
    char buf[DATE_SIZE];
    http_response_date_now(buf, sizeof(buf));
    char date[HEADER_SIZE];
    sprintf(date, "Date: %s\r\n", buf);
    memset(buf, 0, sizeof(buf));

    /* Server header */
    char *signature = NULL;
        /* Retrieve SIGNATURE_ENV */
    signature = getenv(SIGNATURE_ENV);
    char server[HEADER_SIZE];
    sprintf(server, "Server: %s/1.0.1 (Linux)\r\n", signature);

    /* Last Modified + Content-Lenght */
    char last_mod[HEADER_SIZE] = "";
    char last_mod_content[HEADER_SIZE-32] = "";
    char content_len[HEADER_SIZE] = "";
    long con_len;
    if (path) {
        struct stat attr;

        stat(path, &attr);
        strftime(last_mod_content, HEADER_SIZE, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&attr.st_mtime));
        sprintf(last_mod, "Last-Modified: %s\r\n", last_mod_content);

        if(is_script == SCRIPT_NOT_EXECUTED)
            con_len = attr.st_size;
        
        sprintf(content_len, "Content-Length: %ld\r\n", con_len);
    }

    /* Content-Type */
    char content_type[HEADER_SIZE] = "";
    if (ext) {
        char ret[20];
        if (get_content_type(ext, ret) == -1) {
            http_response_set_error(ERR_400, response);
        }
        sprintf(content_type, "Content-Type: %s\r\n", ret);
        memset(ret, 0, sizeof(ret));
    
    /* If HTTP Error */
    } else
        sprintf(content_type, "Content-Type: text/html\r\n");

    if(is_script == SCRIPT_EXECUTED){
        con_len = strlen(response->content);
        sprintf(content_len, "Content-Length: %ld\r\n", con_len);
    }

    /* Poner headers en estructura */
    strcpy(response->headers[0], date);
    strcpy(response->headers[1], server);
    strcpy(response->headers[2], last_mod);
    strcpy(response->headers[3], content_len);
    strcpy(response->headers[4], content_type);
}

void http_response_date(char *buf, size_t buf_len, struct tm *tm)
{
    const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
        "Aug", "Sep", "Oct", "Nov", "Dec"};

    snprintf(buf, buf_len, "%s, %d %s %d %02d:%02d:%02d GMT",
        days[tm->tm_wday], tm->tm_mday, months[tm->tm_mon],
        tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

int http_response_date_now(char *buf, size_t buf_len)
{
    time_t now = time(NULL);
    if (now == -1)
        return -1;

    struct tm *tm = gmtime(&now);
    if (tm == NULL)
        return -1;

    http_response_date(buf, buf_len, tm);
    return 0;
}

char *get_filename_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int get_content_type(char* ext, char* http_formated_type){
    if(STRCMP("html", ext) || STRCMP("htm", ext)
        || STRCMP("py", ext) || STRCMP("php", ext))
        strcpy(http_formated_type, "text/html");
    else if (STRCMP("gif", ext))
        strcpy(http_formated_type, "image/gif");
    else if (STRCMP("jpeg", ext) || STRCMP("jpg", ext))
        strcpy(http_formated_type, "image/jpeg");
    else if (STRCMP("png", ext))
        strcpy(http_formated_type, "image/png");
    else if (STRCMP("mpeg", ext) || STRCMP("mpg", ext))
        strcpy(http_formated_type, "video/mpeg");
    else if (STRCMP("doc", ext) || STRCMP("docx", ext))
        strcpy(http_formated_type, "application/msword");
    else if (STRCMP("pdf", ext))
        strcpy(http_formated_type, "application/pdf");
    else if (STRCMP("css", ext))
        strcpy(http_formated_type, "text/css"); 
    else if (STRCMP("txt", ext))
        strcpy(http_formated_type, "text/plain");
    else{
        LOG_ERR("Extension de archivo MIME no soportada: %s", ext);
        return -1;
    }
    return 0;
}

char* get_args_for_get(char* init_path, char* extracted_args){
    if(!init_path || !extracted_args) return NULL;

    if(strstr(init_path, "?")){
        char* tok;
        char* final_path = malloc(strlen(init_path)+1);
        if(!final_path) return NULL;

        /* Get args */
        tok = strtok(init_path, "?");
        strcpy(final_path, tok);
        memset(extracted_args, 0, sizeof(extracted_args));

        do{
            tok = strtok(NULL, "=");
            tok = strtok(NULL, "&");
            if(tok)
                strcat(extracted_args, tok);
                strcat(extracted_args, " ");
        }while(tok);

        return final_path;
    }

    strcpy(extracted_args, "");
    return init_path;
}

void get_args_for_post(char* args_in, char* args_out){
    char* tok;

    memset(args_out, 0, sizeof(args_out));
    tok = strtok(args_in, "=");
    tok = strtok(NULL, "&");
    if(tok)
        strcat(args_out, tok);
        strcat(args_out, " ");

    while(tok){
        tok = strtok(NULL, "=");
        tok = strtok(NULL, "&");
        if(tok)
            strcat(args_out, tok);
            strcat(args_out, " ");
    }
}

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

    LOG_INFO("Se va a ejecutar el script: %s", exec);

    res_exec = popen(exec, "r");
    if (!res_exec) {
        LOG_ERR("No se ha podido ejecutar el script %s", path);
        /* Enviar error 500 Internal Error */
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
    //pclose(res_exec);
}

char *read_file_from_FILE(FILE *fp)
{
    char *extension = NULL;

    if(!fp) return NULL;

    char *data = calloc(1, sizeof(char));
    if(!data) return NULL;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        data = realloc(data, (strlen(data) + strlen(line) + 1) * sizeof(char));
        if(!data){
            fclose(fp);
            if (line) free(line);
            return NULL;
        }
        strcat(data, line);
    }

    fclose(fp);

    if (line)
        free(line);

    return data;
}

void httpresponse_send_response(Http_request *req, Http_response *res, int cli_fd, char* path, char* ext, char* args_get, char* args_post)
{   
    if(!res || !req) return;

    char responseASCII[CONTENT_SIZE_AUX];  // Contenido ASCII de la response

    if(is_file_script(ext) == 0){
        long res_size;
        char content[CONTENT_SIZE_AUX - 1024];
        
        char *args = malloc(strlen(args_get)+strlen(args_post)+1);
        sprintf(args, "%s%s", args_get, args_post);

        exec_script(cli_fd, path, args, ext, content, &res_size);
        
        /* Guardar en http response para los headers */
        res->content = malloc(strlen(content)+1);
        if(!res->content) return;
        strcpy(res->content, content);

        http_response_set_headers(res, path, ext, SCRIPT_EXECUTED);

        /* Status line := HTTP-Version SP Status-Code SP Reason-Phrase CRLF */
        sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n%s\r\n",
                                res->version, res->code, res->message,
                                res->headers[0],                        // Date
                                res->headers[1],                        // Server
                                res->headers[2],                        // Last-Modified
                                res->headers[3],                        // Content-Length
                                res->headers[4],                        // Content-Type
                                content
                                );
        /* Enviar response por socket de cliente */
        int response_len = strlen(responseASCII);
        if (sendall(cli_fd, responseASCII, &response_len) == -1) {
            perror("send");
            LOG_ERR("No se pudo enviar respuesta HTTP");
            return;
        }
    }else{
        http_response_set_headers(res, path, ext, SCRIPT_NOT_EXECUTED);

        /* Status line := HTTP-Version SP Status-Code SP Reason-Phrase CRLF */
        sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n",
                                res->version, res->code, res->message,
                                res->headers[0],                        // Date
                                res->headers[1],                        // Server
                                res->headers[2],                        // Last-Modified
                                res->headers[3],                        // Content-Length
                                res->headers[4]                        // Content-Type
                                );
        /* Enviar response por socket de cliente */
        int response_len = strlen(responseASCII);
        if (sendall(cli_fd, responseASCII, &response_len) == -1) {
            perror("send");
            LOG_ERR("No se pudo enviar respuesta HTTP");
            return;
        }

        char content_file[CONTENT_SIZE_AUX];
        int ret, file;
        file = open(path, O_RDONLY);
        while ((ret = read(file, content_file, CONTENT_SIZE_AUX)) > 0) {
            sendall(cli_fd, content_file, &ret);
        }
        close(file);
    }
    
    return;
}

void httpresponse_send_error(Http_response *res, int cli_fd)
{   
    if(!res) return;

    char responseASCII[CONTENT_SIZE_AUX];  // Contenido ASCII de la response

    /* Status line := HTTP-Version SP Status-Code SP Reason-Phrase CRLF */
    sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n\r\n%s\r\n",
                            res->version, res->code, res->message,
                            res->headers[0],                        // Date
                            res->headers[1],                        // Server
                            res->headers[2],                        // Last-Modified
                            res->headers[3],                        // Content-Length
                            res->headers[4],                        // Content-Type
                            res->content
                            );
    send(cli_fd, responseASCII, strlen(responseASCII), 0);

    return;
}

void httpresponse_send_options(Http_response *res, int cli_fd)
{   
    if(!res) return;

    char responseASCII[CONTENT_SIZE_AUX];  // Contenido ASCII de la response

    /* Status line := HTTP-Version SP Status-Code SP Reason-Phrase CRLF */
    sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%sContent-Length: 0\r\nAllow: GET, POST, OPTIONS\r\n\r\n",
                            res->version, res->code, res->message,
                            res->headers[0],                        // Date
                            res->headers[1]                         // Server
                            );
    send(cli_fd, responseASCII, strlen(responseASCII), 0);

    return;
}

void http_response_set_error(HTTPErrorCode err_code, Http_response *response)
{
    switch (err_code) {
        case ERR_400: 
            response->code = 400;
            strcpy(response->message, "Bad Request");
            break;
        case ERR_404: 
            response->code = 404;
            strcpy(response->message, "Not Found");
            break;
        case ERR_500: 
            response->code = 500;
            strcpy(response->message, "Internal Error");
            break;
        default:
            return;
    }
    return;
}

Http_response *http_response_get_error_response(HTTPErrorCode err_code)
{
    Http_response *response = httpresponse_init(); 
    if(!response) 
        return NULL;

    http_response_set_error(err_code, response);

    char *html_err = "<html>\n"
                     "<head><title>%d %s</title></head>\n"
                     "<body>\n"
                     "<center><h1>%d %s</h1></center>\n"
                     "<hr><center>Mapache v/1.0.1</center>\n"
                     "</body>\n"
                     "</html>";
    char *html_err_c = calloc(1, strlen(html_err));
    sprintf(html_err_c, html_err, 
            response->code, response->message,
            response->code, response->message);
    
    response->content = html_err_c;

    http_response_set_headers(response, NULL, NULL, -1);
    return response;
}

int is_file_script(char* ext) {
    if(!ext)
        return -1;
    
    if(STRCMP("py", ext) || STRCMP("php", ext))
        return 0;

    return 1;
}
