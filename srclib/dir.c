#include "../include/dir.h"


int path_is_directory(char *path)
{
    if (!path)
        return -1;

    if (path[strlen(path)-1] == '/')
        return 1;
    return 0;
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
                           "<style>table{ font-family:monospace; border-collapse: separate; border-spacing: 40px 5px;}</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<h2>Index of %s</h2>\n"
                    "<hr>\n"
                    "<table>\n"
                        "<tr>\n"
                        "<th>Name</th>\n"
                        "<th>Last Modified</th>\n"
                        "<th>Type</th>\n"
                        "<th>Size</th>\n"
                        "</tr>\n";

    char *index_template_row = "<tr>\n"
                                    "<td><a href=\"%s%s\">%s</a></td>\n"
                                    "<td>Date</td>\n"
                                    "<td>%c</td>\n"
                                    "<td>%d</td>\n"
                                    "</tr>\n";

    char *index_template_end = "</table>\n"
                               "</body>\n"
                               "</html>";

    /* Add start of index */
    size_t needed = snprintf(NULL, 0, index_template, dirpath_short, dirpath_short) + 1;
    html_index = (char*)realloc(html_index, needed);
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
    char type = 0;
    char p_aux[1024] = "";
    while ((dir = readdir(d)) != NULL) {
        if (STRCMP(dir->d_name, ".") || STRCMP(dir->d_name, ".."))
            continue;

        sprintf(p_aux, "%s%s", dirpath, dir->d_name);
        if (stat(dirpath, &statbuf) != 0) {
            perror("stat");
            return NULL;
        }
        if (S_ISREG(statbuf.st_mode))
            type = 'f';
        else if (S_ISDIR(statbuf.st_mode))
            type = 'D';
        statbuf = emptystat;

        needed_row = snprintf(NULL, 0, index_template_row, dirpath, dir->d_name, dir->d_name, type, dir->d_reclen) + 1;
        html_index_row = (char*)realloc(html_index_row, needed_row);
        needed += needed_row;
        html_index = (char*)realloc(html_index, needed);
        sprintf(html_index_row, index_template_row, request_path, dir->d_name, dir->d_name, type, dir->d_reclen);
        strcat(html_index, html_index_row);
        free(html_index_row);
        html_index_row = NULL;
    }
    closedir(d);

    /* Add end of index HTML */
    size_t needed_end = snprintf(NULL, 0, index_template_end, NULL) + 1;
    html_index_end = (char*)realloc(html_index_end, needed_end);
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
