#include "../include/dir.h"
#include "../include/mime.h"


int path_is_directory(char *path)
{
    if (!path) return -1;

    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        perror("stat");
        return -1;
    }

    return S_ISDIR(statbuf.st_mode);
}

char* get_directory_as_index(char *dirpath)
{
    char *html_index = NULL;
    char *html_index_row = NULL;
    char *html_index_end = NULL;

    char *dirpath_short = get_shortened_dirpath(dirpath);
    if (dirpath_short == NULL) {
        LOG_ERR("Error generating shortened dirpath");
        return NULL;
    }

    /* Get original requested path (w/o absolute route) */
    char* request_path = getenv("REQ_PATH_ENV");
    if (request_path == NULL)
        return NULL;

    char *index_template = "<html>\n"
                    "<head>"
                           "<title>Index of %s</title>\n"
                           "<style>\n"
                                "h2{ font-family: sans-serif; }\n"
                                "table{ font-family:monospace; border-collapse: separate; border-spacing: 40px 5px;}\n"
                           "</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<h2>Index of %s</h2>\n"
                    "<hr>\n"
                    "<table>\n"
                        "<tr>\n"
                        "<th></th>"
                        "<th>Name</th>\n"
                        "<th>Last Modified</th>\n"
                        "<th>Type</th>\n"
                        "<th>Size</th>\n"
                        "</tr>\n";

    char *index_template_row = "<tr>\n"
                                    "<td><img src=\"%s\" height=25vh></td>\n"
                                    "<td><a href=\"%s%s%s\">%s</a></td>\n"      // path + slash¿ + file
                                    "<td>Date</td>\n"
                                    "<td>%c</td>\n"
                                    "<td>%d</td>\n"
                                    "</tr>\n";

    char *index_template_end = "</table>\n"
                               "</body>\n"
                               "</html>";

    /* Add start of index */
    size_t needed = snprintf(NULL, 0, index_template, dirpath_short, dirpath_short) + 1;
    html_index = (char*)malloc(needed);
    sprintf(html_index, index_template, dirpath_short, dirpath_short);

    /* Add files as rows */
    DIR *d;
    struct dirent *dir;
    d = opendir(dirpath);
    if (!d) {
        LOG_ERR("Could not open directory %s", dirpath);
        return NULL;
    }
    size_t needed_row = 0;
    struct stat statbuf, emptystat;
    char type = FILETYPE;
    char p_aux[1024] = "";
    char* icon_url = NULL;
    char* slash = "";
    while ((dir = readdir(d)) != NULL) {
        /* Skip parent & current directory links */
        if (STRCMP(dir->d_name, ".") || STRCMP(dir->d_name, ".."))
            continue;

        /* Decide if it's a file or a directory */
        sprintf(p_aux, "%s%s", dirpath, dir->d_name);
        if (stat(p_aux, &statbuf) != 0) {
            perror("stat");
            return NULL;
        }
        if (S_ISREG(statbuf.st_mode))
            type = FILETYPE;
        else if (S_ISDIR(statbuf.st_mode))
            type = DIRTYPE;

        /* Decide icon to show */
        icon_url = get_icon_url_from_ext(get_filename_extension(p_aux), type);

        /* Append slash if path doesn't end in / */
        slash = "";
        if (request_path[strlen(request_path)-1] != '/')
            slash = "/";

        /* Add row to HTML table index */
        needed_row = snprintf(NULL, 0, index_template_row, icon_url, request_path, slash, dir->d_name, dir->d_name, type, dir->d_reclen) + 1;
        html_index_row = (char*)malloc(needed_row);
        needed += needed_row;
        html_index = (char*)realloc(html_index, needed);
        sprintf(html_index_row, index_template_row, icon_url, request_path, slash, dir->d_name, dir->d_name, type, dir->d_reclen);
        strcat(html_index, html_index_row);
        free(html_index_row);
        html_index_row = NULL;
    }
    closedir(d);

    /* Add end of index HTML */
    size_t needed_end = snprintf(NULL, 0, index_template_end, NULL) + 1;
    html_index_end = (char*)malloc(needed_end);
    needed += needed_end;
    html_index = (char*)realloc(html_index, needed);
    sprintf(html_index_end, index_template_end, NULL);
    strcat(html_index, html_index_end);
    free(html_index_end);

    return html_index;
}

char* get_shortened_dirpath(char *dirpath)
{
    char *server_root = NULL;
    /* Retrieve shortened ROOT_SHORT env var */
    server_root = getenv(ROOT_SHORT);
    if (server_root == NULL)
        return NULL;

    char *shortened = strstr(dirpath, server_root);
    return shortened;
}

char* get_icon_url_from_ext(char* ext, char type)
{
    if (type != DIRTYPE && type != FILETYPE)
        return UNK_EXT_ICON;

    if (type == DIRTYPE)
        return DIR_ICON;

    if (STRCMP(ext, "html") || STRCMP(ext, "htm"))
        return HTML_FILE_ICON;
    else if (STRCMP(ext, "css"))
        return HTML_FILE_ICON;
    else if (STRCMP(ext, "c"))
        return C_FILE_ICON;
    else if (STRCMP(ext, "c++") || STRCMP(ext, "cpp"))
        return CPP_FILE_ICON;
    else if (STRCMP(ext, "py"))
        return PY_FILE_ICON;
    else if (STRCMP(ext, "php"))
        return PHP_FILE_ICON;
    else if (STRCMP(ext, "jpg") || STRCMP(ext, "jpeg") || STRCMP(ext, "png") || STRCMP(ext, "svg"))
        return IMG_FILE_ICON;
    else if (STRCMP(ext, "mp3") || STRCMP(ext, "mp4") || STRCMP(ext, "mpeg") || STRCMP(ext, "mpeg3") || STRCMP(ext, "mpeg4") || STRCMP(ext, "avi"))
        return VIDEO_FILE_ICON;
    else if (STRCMP(ext, "gif"))
        return GIF_FILE_ICON;
    else if (STRCMP(ext, "pdf"))
        return PDF_FILE_ICON;
    else if (STRCMP(ext, "txt") || STRCMP(ext, "dat") || STRCMP(ext, "conf") || STRCMP(ext, "csv") || STRCMP(ext, "tsv"))
        return TXT_FILE_ICON;
    else if (STRCMP(ext, "doc") || STRCMP(ext, "docx"))
        return DOC_FILE_ICON;

    return UNK_EXT_ICON;
}




