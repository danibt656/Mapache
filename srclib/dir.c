#include "../include/dir.h"


int path_is_directory(char *path)
{
    if (!path)
        return -1;

    LOG_INFO("Given path: %s", path);
    if (path[strlen(path)-1] == '/')
        return 1;
    return 0;
}

char* get_directory_as_index(char *dirpath)
{
    char *html_index = NULL;
    char *index_template = "<html>\n"
                    "<head><title>Index of %s</title></head>\n"
                    "<body>\n"
                    "<center><h1>Index of %s</h1></center>\n"
                    "<hr>\n"
                    "<p>Here goes a directory</p>\n"
                    "<table>\n"
                        "<tr>\n"
                        "<th>Name</th>\n"
                        "<th>Last Modified</th>\n"
                        "<th>Size</th>\n"
                        "</tr>\n"
                        "<tr>\n"
                        "<td>Alfreds Futterkiste</td>\n"
                        "<td>Maria Anders</td>\n"
                        "<td>Germany</td>\n"
                        "</tr>\n"
                        "<tr>\n"
                        "<td>Centro comercial Moctezuma</td>\n"
                        "<td>Francisco Chang</td>\n"
                        "<td>Mexico</td>\n"
                        "</tr>\n"
                    "</table>\n"
                    "</body>\n"
                    "</html>";

    html_index = (char*)realloc(html_index, strlen(index_template) + 2*strlen(dirpath));
    sprintf(html_index, index_template, dirpath, dirpath);

    DIR *d;
    struct dirent *dir;
    d = opendir(dirpath);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }

    return html_index;
}