#ifndef _MIME_H
#define _MIME_H

#include "libsocket.h"

char *get_filename_extension(char *filename);

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
 * Check if file extension is a supported script.
 * 
 * @param ext   File extension
 * 
 * @return  0 if it's a script
 *          1 if not 
 *         -1 if error
 */
int is_file_script(char* ext);

#endif // _MIME_H