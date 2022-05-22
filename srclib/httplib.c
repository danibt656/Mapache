#include "../include/httplib.h"


// ****************************************************************************
//                                 HTTP Request
// ****************************************************************************

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

__attribute__((always_inline)) inline void httprequest_free(Http_request* req)
{
    if(req)
        free(req);
}

Http_request *httprequest_parse_and_map(void* _cli_fd)
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

    char* is_tls_enabled = NULL;
    is_tls_enabled = getenv(TLS_EN_ENV);
    
    while (1) {
        /* read request */
        if (is_tls_enabled == NULL)
            while ((rret = read((intptr_t)_cli_fd, buff + buflen, sizeof(buff) - buflen)) == -1 && errno == EINTR);
        else
            while ((rret = SSL_read((SSL*)_cli_fd, buff + buflen, sizeof(buff) - buflen)) == -1 && errno == EINTR);
        
        if (rret <= 0)
            return NULL;
        prevbuflen = buflen;
        buflen += rret;

        /* parse request */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(buff, buflen, &method, &method_len, &path, &path_len,
                                &minor_version, headers, &num_headers, prevbuflen);
        
        if (pret > 0) {
            LOG_INFO("HTTP Request parsed");
            break;
        }
        
        else if (pret == -1) {
            return NULL;
        }

        /* incomplete request, continue loop */
        assert(pret == -2);

        if (buflen == sizeof(buff))
            return NULL;
    }

    /* Enter data into struct & valdiate it */
    int validate_status = 0;
    if ((validate_status = httprequest_set_all(request, buff, method_len,
                            method, path_len,path, minor_version,
                            pret, num_headers, headers)) != HTTP_VALID)
    {
        Http_response *error = http_response_get_error_response(validate_status);
        httpresponse_send_error(error, _cli_fd);
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

    /* Method */
    sprintf(aux, "%.*s", (int) method_len, method);
        /* Check method support */
    if (check_http_method_support(aux) == -1) {
        LOG_ERR("501 Method %s not implemented", aux);
        return ERR_501;
    }
    strcpy(req->method, aux);
    memset(aux, 0, strlen(aux));

    /* Version */
        /* Support version check */
    if (version != HTTP_V_0 && version != HTTP_V_1) {
        LOG_ERR("HTTP version is incompatible. Only accepts HTTP/1.0 or HTTP/1.1.");
        return ERR_505;
    }
    req->version = version;

    /* Path */
    char aux2[AUX_SIZE];
    sprintf(aux2, "%.*s", (int) path_len, path);
        /* Set request path env variable */
    if (setenv("REQ_PATH_ENV", aux2, 1) < 0) {
        perror("setenv");
        LOG_ERR("Couldn't set REQ_PATH_ENV to \'%s\'", aux2);
        exit(EXIT_FAILURE);
    }
        /* Concatenate with server root (absolute path) */
    path_root = getenv(ROOT_ENV);
    sprintf(aux, "%s%s", path_root, aux2);
    strcpy(req->path, aux);
    memset(aux, 0, strlen(aux));

    /* Size */
    req->size = size;

    /* Headers */
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

    LOG_INFO("HTTP Request valid");
    return HTTP_VALID;
}

int check_http_method_support(char *method)
{
    if (STRCMP(method, "GET") ||
        STRCMP(method, "POST") ||
        STRCMP(method, "HEAD") ||
        STRCMP(method, "OPTIONS")
        )
        return 0;
    return -1;
}

void httprequest_print(Http_request* req)
{
    LOG_INFO("Size: %d bytes", req->size);
    LOG_INFO("Command: %s", req->method);
    LOG_INFO("Path: %s", req->path);
    LOG_INFO("HTTP version: 1.%d", req->version);

    LOG_INFO("%d headers:", req->num_headers);
    for (int i = 0; i < req->num_headers; i++) {
        LOG_INFO("%.*s: %.*s", (int)req->headers[i].name_len, (char*)req->headers[i].name, \
                                 (int)req->headers[i].value_len, (char*)req->headers[i].value);
    }
    if(strstr(req->method, "POST"))
        LOG_INFO("POST arguments: %s\n", req->post_args);
}

// ****************************************************************************
//                                 HTTP RESPONSE
// ****************************************************************************

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

__attribute__((always_inline)) inline void httpresponse_free(Http_response* res)
{
    if (res->content)
        free(res->content);
    free(res);
}

void http_response_eval_request(Http_request *request, void* cli_fd)
{
    char args_for_post[AUX_SIZE]="";

    Http_response *response = httpresponse_init(); 
    if(!response) return;

    if (STRCMP(request->method, "OPTIONS")) {
        response->content = NULL;
        http_response_set_headers(response, NULL, NULL, -1);
        httpresponse_send_options(response, cli_fd);
        httpresponse_free(response);
        return;
    }

    char* args_for_get = malloc(strlen((request->path)+1)*sizeof(char*));
    if(!args_for_get){
        httpresponse_free(response);
        return;
    }
    /* Extract args from path (if there are any) */
    char *path;
    if(!(path = get_args_for_get(request->path, args_for_get))){
        free(args_for_get);
        httpresponse_free(response);
        return;
    }

    if (access(path, F_OK) != 0) {
        LOG_ERR("404 \'%s\' not found", path);
        httpresponse_free(response);
        free(args_for_get);
        Http_response *error = http_response_get_error_response(ERR_404);
        httpresponse_send_error(error, cli_fd);
        httpresponse_free(error);
        return;
    }

    /* If path is a directory, show an Index page */
    if (path_is_directory(request->path)) {
        size_t len = strlen(request->path);
        /* If it's a directory, enforce that ends with '/' */
        if (request->path[len-1] != '/') {
            char _s = '/';
            strncat(request->path, &_s, 1);
        }
        char *index_dir = get_directory_as_index(request->path);
        set_index_dir_response(response, index_dir);
        httpresponse_send_response(request, response, cli_fd, request->path, DIR_CODE, args_for_get, args_for_post);
        free(args_for_get);
        return;
    }

    /* Get extension from request */
    char *ext = get_filename_extension(path);
    
    /* Send response as ASCII */
    if (STRCMP(request->method, "POST"))
        get_args_for_post(request->post_args, args_for_post);
    httpresponse_send_response(request, response, cli_fd, path, ext, args_for_get, args_for_post);
    free(args_for_get);
    //httpresponse_free(response);
    LOG_INFO("HTTP Response sent");
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
        /* Retrieve SIGNATURE_ENV */
    char *signature = NULL;
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

    /* Set headers in struct */
    strcpy(response->headers[0], date);
    strcpy(response->headers[1], server);
    strcpy(response->headers[2], last_mod);
    strcpy(response->headers[3], content_len);
    strcpy(response->headers[4], content_type);
}

__attribute__((always_inline)) inline int http_response_date_now(char *buf, size_t buf_len)
{
    time_t now = time(NULL);
    if (now == -1)
        return -1;

    struct tm *tm = gmtime(&now);
    if (tm == NULL)
        return -1;

    const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    snprintf(buf, buf_len, "%s, %d %s %d %02d:%02d:%02d GMT",
        days[tm->tm_wday], tm->tm_mday, months[tm->tm_mon],
        tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return 0;
}

char* get_args_for_get(char* init_path, char* extracted_args)
{
    if(!init_path || !extracted_args) return NULL;

    if(strstr(init_path, "?")){
        char* tok;
        char* final_path = malloc(strlen(init_path)+1);
        if(!final_path) return NULL;

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

void get_args_for_post(char* args_in, char* args_out)
{
    char* tok;

    memset(args_out, 0, sizeof(args_out));
    tok = strtok(args_in, "=");
    tok = strtok(NULL, "&");
    if (tok)
        strcat(args_out, tok);
        strcat(args_out, " ");

    while (tok){
        tok = strtok(NULL, "=");
        tok = strtok(NULL, "&");
        if (tok)
            strcat(args_out, tok);
            strcat(args_out, " ");
    }
}

void httpresponse_send_response(Http_request *req, Http_response *res, void* cli_fd, char* path, char* ext, char* args_get, char* args_post)
{   
    if(!res || !req) return;

    char responseASCII[CONTENT_SIZE_AUX];

    if (is_file_script(ext) == 0) {
        LOG_INFO("Sending script...");

        long res_size;
        char content[CONTENT_SIZE_AUX - 1024];
        
        char *args = malloc(strlen(args_get)+strlen(args_post)+1);
        sprintf(args, "%s%s", args_get, args_post);
        exec_script(cli_fd, path, args, ext, content, &res_size);
        
        res->content = malloc(strlen(content)+1);
        if(!res->content) {
            httpresponse_free(res);
            return;
        }
        strcpy(res->content, content);

        http_response_set_headers(res, path, ext, SCRIPT_EXECUTED);

        sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n%s\r\n",
                                res->version, res->code, res->message,
                                res->headers[0],                        // Date
                                res->headers[1],                        // Server
                                res->headers[2],                        // Last-Modified
                                res->headers[3],                        // Content-Length
                                res->headers[4],                        // Content-Type
                                content
                                );
        int response_len = strlen(responseASCII);
        if (sendall(cli_fd, responseASCII, &response_len) == -1) {
            perror("send");
            LOG_ERR("Couldn't send HTTP response");
            httpresponse_free(res);
            return;
        }
    } else if (STRCMP(ext, DIR_CODE)) {
        LOG_INFO("Sending directory index...");
        if (!STRCMP(req->method, "HEAD")) {
            sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n%s\r\n",
                                    res->version, res->code, res->message,
                                    res->headers[0],                        // Date
                                    res->headers[1],                        // Server
                                    res->headers[2],                        // Last-Modified
                                    res->headers[3],                        // Content-Length
                                    res->headers[4],                        // Content-Type
                                    res->content
                                    );
        } else {
            sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n",
                                    res->version, res->code, res->message,
                                    res->headers[0],                        // Date
                                    res->headers[1],                        // Server
                                    res->headers[2],                        // Last-Modified
                                    res->headers[3],                        // Content-Length
                                    res->headers[4]                         // Content-Type
                                    );
        }
        int response_len = strlen(responseASCII);
        if (sendall(cli_fd, responseASCII, &response_len) == -1) {
            perror("send");
            LOG_ERR("Couldn't send HTTP response");
            httpresponse_free(res);
            return;
        }
    } else {
        LOG_INFO("Sending page...");
        http_response_set_headers(res, path, ext, SCRIPT_NOT_EXECUTED);
        sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n",
                                res->version, res->code, res->message,
                                res->headers[0],                        // Date
                                res->headers[1],                        // Server
                                res->headers[2],                        // Last-Modified
                                res->headers[3],                        // Content-Length
                                res->headers[4]                         // Content-Type
                                );
        int response_len = strlen(responseASCII);
        if (sendall(cli_fd, responseASCII, &response_len) == -1) {
            perror("send");
            LOG_ERR("Couldn't send HTTP response");
            httpresponse_free(res);
            return;
        }

        if (!STRCMP(req->method, "HEAD")) {
            char content_file[CONTENT_SIZE_AUX];
            int ret, file;
            file = open(path, O_RDONLY);
            while ((ret = read(file, content_file, CONTENT_SIZE_AUX)) > 0) {
                sendall(cli_fd, content_file, &ret);
            }
            close(file);
        }
    }
    httpresponse_free(res);
    return;
}

void httpresponse_send_error(Http_response *res, void* cli_fd)
{   
    if(!res) return;

    char responseASCII[CONTENT_SIZE_AUX];

    sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%s%s%s%s\r\n\r\n%s\r\n",
                            res->version, res->code, res->message,
                            res->headers[0],                        // Date
                            res->headers[1],                        // Server
                            res->headers[2],                        // Last-Modified
                            res->headers[3],                        // Content-Length
                            res->headers[4],                        // Content-Type
                            res->content
                            );
    Send(cli_fd, responseASCII, strlen(responseASCII));
}

void httpresponse_send_options(Http_response *res, void* cli_fd)
{   
    if(!res) return;

    char responseASCII[CONTENT_SIZE_AUX];

    sprintf(responseASCII, "HTTP/1.%d %d %s\r\n%s%sContent-Length: 0\r\nAllow: GET, POST, OPTIONS, HEAD\r\n\r\n",
                            res->version, res->code, res->message,
                            res->headers[0],                        // Date
                            res->headers[1]                         // Server
                            );
    Send(cli_fd, responseASCII, strlen(responseASCII));
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
            strcpy(response->message, "Internal Server Error");
            break;
        case ERR_501: 
            response->code = 501;
            strcpy(response->message, "Not Implemented");
            break;
        case ERR_505: 
            response->code = 505;
            strcpy(response->message, "HTTP Version Not Supported");
            break;
        default:
            return;
    }
}

Http_response *http_response_get_error_response(HTTPErrorCode err_code)
{
    Http_response *response = httpresponse_init(); 
    if(!response) 
        return NULL;

    http_response_set_error(err_code, response);

    char* signature = getenv(SIGNATURE_ENV);

    char *html_err = "<html>\n"
                     "<head><title>%d %s</title></head>\n"
                     "<body>\n"
                     "<center><h1>%d %s</h1></center>\n"
                     "<hr><center>%s v/2.3.0</center>\n"
                     "</body>\n"
                     "</html>";
    char *html_err_c = calloc(1, strlen(html_err)*2);
    sprintf(html_err_c, html_err,
            response->code, response->message,
            response->code, response->message,
            signature);
    
    response->content = html_err_c;

    http_response_set_headers(response, NULL, NULL, -1);
    return response;
}

void set_index_dir_response(Http_response *response, char* index_dir)
{
    if (!response || !index_dir)
        return;
    
    response->content = index_dir;
    http_response_set_headers(response, NULL, NULL, -1);
}
