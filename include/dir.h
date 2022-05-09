#ifndef DIR_H
#define DIR_H

#include "liblog.h"
#include "httplib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define DIRTYPE 'D'
#define FILETYPE 'f'

/* Wikimedia Nuvola-like file icons */
#define DIR_ICON            "https://upload.wikimedia.org/wikipedia/commons/3/34/Folder_with_files.png"
#define UNK_EXT_ICON        "https://upload.wikimedia.org/wikipedia/commons/3/37/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon.png"

#define C_FILE_ICON         "https://upload.wikimedia.org/wikipedia/commons/b/bb/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-c.png"
#define CPP_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/c/cd/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-cpp.png"
#define HEAD_FILE_ICON      "https://upload.wikimedia.org/wikipedia/commons/c/ce/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-h.png"
#define PY_FILE_ICON        "https://upload.wikimedia.org/wikipedia/commons/3/36/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-py.png"
#define PHP_FILE_ICON       "https://www.theasgroups.com/images/subpageimages/php-mysql.png"
#define HTML_FILE_ICON      "https://upload.wikimedia.org/wikipedia/commons/7/78/Fairytale_html.png"
#define CSS_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/2/26/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-css.png"

#define IMG_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/thumb/9/95/File_equals_icon.svg/640px-File_equals_icon.svg.png"
#define VIDEO_FILE_ICON     "https://upload.wikimedia.org/wikipedia/commons/0/02/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-avi.png"
#define GIF_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/8/85/Farm-Fresh_file_extension_gif.png"

#define PDF_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/a/a4/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-pdf.png"
#define DOC_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/f/f8/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-doc.png"
#define TXT_FILE_ICON       "https://upload.wikimedia.org/wikipedia/commons/7/75/Nuvola-inspired_File_Icons_for_MediaWiki-fileicon-txt.png"

/**
 * @brief Determines wether a path is a directory or a file
 * 
 * @param path  The path to evaluate
 * 
 * @return 1 if it's a path, 0 if it's a file, -1 if error
 */
int path_is_directory(char *path);

/**
 * @brief Get the directory as index HTML response, filling up its contents
 *
 * @param response  The HTTP response struct to fill
 * @param dirpath   The path to the directory
 *
 * @return The directory as an Index HTML page
 */
char* get_directory_as_index(char *dirpath);

/**
 * @brief Get the directory path starting from Server's root
 *
 * @param dirpath   The directory path to shorten
 *
 * @return The shortened directory path
 */
char* get_shortened_dirpath(char *dirpath);

/**
 * @brief Get appropiate icon from Wikimedia according to extension
 *
 * @param ext   The file's extension
 * @param type  'f' for Files, 'D' for directories
 *
 * @return The Wikimedia icon's URL
 */
char* get_icon_url_from_ext(char* ext, char type);

#endif
