#include "../include/mime.h"
#include "../include/httplib.h"


char *get_filename_extension(char *filename)
{
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int get_content_type(char* ext, char* http_formated_type)
{
    /* HTML pages, scripts & directory indexes */
    if(STRCMP("html", ext) || STRCMP("htm", ext)
        || STRCMP("py", ext) || STRCMP("php", ext)
        || STRCMP(DIR_CODE, ext))
        strcpy(http_formated_type, "text/html");
    /* GIFs */
    else if (STRCMP("gif", ext))
        strcpy(http_formated_type, "image/gif");
    /* JPGs / JPEGs */
    else if (STRCMP("jpeg", ext) || STRCMP("jpg", ext))
        strcpy(http_formated_type, "image/jpeg");
    /* PNGs */
    else if (STRCMP("png", ext))
        strcpy(http_formated_type, "image/png");
    /* Video */
    else if (STRCMP("mpeg", ext) || STRCMP("mpg", ext))
        strcpy(http_formated_type, "video/mpeg");
    /* Word documents */
    else if (STRCMP("doc", ext) || STRCMP("docx", ext))
        strcpy(http_formated_type, "application/msword");
    /* PDFs */
    else if (STRCMP("pdf", ext))
        strcpy(http_formated_type, "application/pdf");
    /* CSS style sheets */
    else if (STRCMP("css", ext))
        strcpy(http_formated_type, "text/css"); 
    /* Other text formats */
    else if (STRCMP("txt", ext))
        strcpy(http_formated_type, "text/plain");
    /* Unsupported formats */
    else{
        LOG_ERR("Extension de archivo MIME no soportada: %s", ext);
        return -1;
    }
    return 0;
}

int is_file_script(char* ext)
{
    if(!ext)
        return -1;
    
    if(STRCMP("py", ext) || STRCMP("php", ext))
        return 0;

    return 1;
}
