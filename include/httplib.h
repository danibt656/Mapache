#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "picohttpparser.h"
#include "libsocket.h"
#include "liblog.h"

#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>

/* HTTP versions (1.0, 1.1) */
#define HTTP_V_0 0
#define HTTP_V_1 1

#define HTTP_VALID 0
#define HTTP_INVALID 1

#define SCRIPT_EXECUTED 0
#define SCRIPT_NOT_EXECUTED 1

#define METHOD_SIZE 16
#define DATE_SIZE 32
#define PATH_SIZE 128
#define SCRIPT_CMD_SIZE 128
#define NUM_HEADERS 128             // Max number of allowed headers
#define TAM_HEADER 2048
#define POST_ARGS_SIZE 1024
#define MAX_ARGS_GET_METHOD 16      // MAx number of args for get method
#define CONTENT_SIZE_AUX 32768

#define EXTENSION_SIZE 5    // File extension
#define HEADER_SIZE 128     // HTTP Reply Headers size

// ****************************************************************************
//                                 HTTP REQUEST
// ****************************************************************************
typedef struct _Http_request Http_request;

Http_request* httprequest_init();

void httprequest_free(Http_request* req);

Http_request *httprequest_parse_and_map(int cli_fd);

/**
 * Insert parameters into HTTP Request struct
 * 
 * @param req           HTTP Request struct
 * @param buff          HTTP Request ascii
 * @param method_len    Method length
 * @param method        Method (GET, POST, OPTIONS)
 * @param path_len      Path length
 * @param path          Path (URL/URI)
 * @param version       HTTP Version
 * @param size          Request size
 * @param num_headers   Number of headers
 * @param headers       Headers
 * 
 * @return  HTTP_VALID if request is valid, HTTP_INVALID if not
*/
int httprequest_set_all(Http_request* req,
                         char* buff,
                         int method_len,
                         const char* method,
                         int path_len,
                         const char* path,
                         int version,
                         int size,
                         int num_headers, 
                         struct phr_header *headers);

int check_http_method(char *method);

void httprequest_print(Http_request* req);

// ****************************************************************************
//                                 HTTP RESPONSE
// ****************************************************************************
typedef struct _Http_response Http_response;


Http_response* httpresponse_init();

void httpresponse_free(Http_response* res);

typedef enum {
    ERR_400,      // 400 Bad Request
    ERR_404,      // 404 Not Found
    ERR_500       // 500 Internal Error
} HTTPErrorCode;

/**
 * Eval HTTP request & send HTTP response
 * 
 * @param request   HTTP Request struct
 * @param cli_fd    Client connection socket
 */
void http_response_eval_request(Http_request *request, int cli_fd);

/**
 * Get dates in HTTP time-date format
 * 
 * @param buf       Date-storing buffer
 * @param buf_len   Buffer length
 * @param tm        Time data structure (<time.h>)
 */
void http_response_date(char *buf, size_t buf_len, struct tm *tm);
int http_response_date_now(char *buf, size_t buf_len);

char *get_filename_extension(char *filename);

char *read_file(const char *filename);

char *read_file_from_FILE(FILE *fp);

/**
 * Extract arguments from GET path & return path w/o args
 * 
 * @param init_path         Path with args
 * @param extracted_args    Args extracted from path
 * 
 * @return "Clean path" (with no GET args)
*/
char* get_args_for_get(char* init_path, char* extracted_args);

/**
 * Given an args string (e.g.: _=_&_=_), split its values.
 *
 * 
 * @param args_in   Unparsed args string
 * @param args_out  Parsed args
 * 
*/
void get_args_for_post(char* args_in, char* args_out);

/**
 * Given a file extension, return its MIME Content Type
 * 
 * @param ext                   Extension
 * @param http_formated_type    HTTP Content-Type
 * 
 * @return  0 si soportada.
 *          -1 de lo contrario.
*/
int get_content_type(char* ext, char* http_formated_type);

/**
 * Set headers for HTTP response
 * 
 * @param response  HTTP response struct
 * @param path      Path for headers
 * @param ext       Extension for Content-Type
 * @param is_script Wether a script has been executed
 * 
*/
void http_response_set_headers(Http_response* response, char *path, char *ext, int is_script);

/**
 * Execute a script.
 * 
 * Must be given:
 *  - path  -> Script path without arguments
 *  - ext   -> Script extension (lang)
 *  - args  -> Script arguments
 * 
 * These arguments will be filled:
 *  - to_fill_content   -> Script execution result
 *  - size              -> Result size in bytes
 * 
 * @param cli_fd            Client connection socket
 * @param path              Relative path of script
 * @param args              Script args
 * @param ext               Script file extension (lang)
 * @param to_fill_content   Script execution script
 * @param size              Result size in bytes
 * 
*/
void exec_script(int cli_fd, char* path, char* args, char* ext, char* to_fill_content, long* size);

/**
 * Check if file extension is a supported script.
 * 
 * @param ext   File extension
 * 
 * @return  0 if it's a script
 *          1 if not 
 *         -1 if error
 */
int is_file_script(char* ext);

/**
 * Send HTTP response
 * Convert HTTP response to ASCII and send it by TCP
 * 
 * @param req       HTTP request struct
 * @param res       HTTP response struct
 * @param cli_fd    Client connection socket
 * @param args_get  Path args (for GET)
 * @param args_post Body args (for POST)
 */
void httpresponse_send_response(Http_request *req, Http_response *res, int cli_fd, char* path, char* ext, char* args_get, char* args_post);

/**
 * Get HTTP error code and put it into response struct
 * 
 * @param err_code      Response error code (400, 404...)
 * @param response      HTTP response struct
 */
void http_response_set_error(HTTPErrorCode err_code, Http_response *response);

/**
 * Generate HTML error page with code.
 * 
 * @param err_code  Error response code (400, 404...)
 */
Http_response *http_response_get_error_response(HTTPErrorCode err_code);

/**
 * Send an error HTTP response
 * 
 * @param res       HTTP Response struct
 * @param cli_fd    Client connection socket
 */
void httpresponse_send_error(Http_response *res, int cli_fd);

/**
 * Send response to OPTIONS method
 * 
 * @param res       HTTP Response struct
 * @param cli_fd    Client connection socket
 */
void httpresponse_send_options(Http_response *res, int cli_fd);

#endif