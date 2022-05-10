#include "../include/io.h"


char *read_file(const char *filename)
{
    char *extension = NULL;

    if(!filename) return NULL;

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        LOG_ERR("Couldn't open file %s", filename);
        return NULL;
    }
    
    char *data = calloc(1, sizeof(char));
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        data = realloc(data, (strlen(data) + strlen(line) + 1) * sizeof(char));
        strcat(data, line);
    }

    fclose(fp);

    if (line)
        free(line);

    return data;
}

char *read_file_from_FILE(FILE *fp)
{
    if(!fp)
        return NULL;

    char *data = calloc(1, sizeof(char));
    if(!data)
        return NULL;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        data = realloc(data, (strlen(data) + strlen(line) + 1) * sizeof(char));
        if(!data){
            fclose(fp);
            if (line)
                free(line);
            return NULL;
        }
        strcat(data, line);
    }

    fclose(fp);

    if (line)
        free(line);
    LOG_INFO("DATA: %s", data);

    return data;
}
