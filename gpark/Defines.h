

#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <iostream>

#define CONSOLE_COLOR_END               "\033[0m"

#define CONSOLE_CLEAR_LINE              "\033[2K"

#define CONSOLE_COLOR_FONT_BLACK        "\033[30m"
#define CONSOLE_COLOR_FONT_RED          "\033[31m"
#define CONSOLE_COLOR_FONT_GREEN        "\033[32m"
#define CONSOLE_COLOR_FONT_YELLOW       "\033[33m"
#define CONSOLE_COLOR_FONT_BLUE         "\033[34m"
#define CONSOLE_COLOR_FONT_PURPLE       "\033[35m"
#define CONSOLE_COLOR_FONT_CYAN         "\033[36m"
#define CONSOLE_COLOR_FONT_WHITE        "\033[37m"

#define CONSOLE_COLOR_BG_BLACK          "\033[40m"
#define CONSOLE_COLOR_BG_DARK_RED       "\033[41m"
#define CONSOLE_COLOR_BG_GREEN          "\033[42m"
#define CONSOLE_COLOR_BG_YELLOW         "\033[43m"
#define CONSOLE_COLOR_BG_BLUE           "\033[44m"
#define CONSOLE_COLOR_BG_PURPLE         "\033[45m"
#define CONSOLE_COLOR_BG_CYAN           "\033[46m"
#define CONSOLE_COLOR_BG_WHITE          "\033[47m"

#define CONSOLE_COLOR_FOLDER            "\033[34m"
#define CONSOLE_COLOR_WHITE_BLACK       "\033[30;47m"

#define GPARK_PATH_HOME                 "_gpark"
#define GPARK_PATH_INDEX                GPARK_PATH_HOME "/index"
#define GPARK_PATH_DB                   GPARK_PATH_HOME "/db"
#define GPARK_PATH_IGNORE               GPARK_PATH_HOME "/ignore"
#define GPARK_PATH_MISS_IGNORE          GPARK_PATH_HOME "/missignore"

#define STORAGE_KB          1024
#define STORAGE_MB          1048576
#define STORAGE_GB          1073741824
#define STORAGE_TB          1099511627776

#define BIG_FILE_SIZE       524288000           // 500MB

#define FORMAT_FILESIZE_BUFFER_LENGTH   50

#define FULLPATH_DEFAULT_BUFFER_LENGTH      1024

#define FILE_BOOL_LENGTH        1
#define FILE_ID_LENGTH          8
#define FILE_FILESIZE_LENGTH    8
#define FILE_MTIME_LENGTH       8
#define SHA1_DIGEST_LENGTH     20
#define FILE_NAMESIZE_LENGTH    4

// len, id, parent_id, isFolder, fileSize, mtime, sha1, nameSize, (name), isSoftLink, isFile
#define FILE_BASIC_LENGTH       FILE_FILESIZE_LENGTH + FILE_ID_LENGTH + FILE_ID_LENGTH + FILE_BOOL_LENGTH + FILE_FILESIZE_LENGTH + FILE_MTIME_LENGTH + SHA1_DIGEST_LENGTH + FILE_NAMESIZE_LENGTH + FILE_BOOL_LENGTH + FILE_BOOL_LENGTH

#define DB_VERSION_V1               1
#define DB_VERSION_V2               2

#define GAssert(expression, format, ...) \
if (!(expression)) { \
printf(CONSOLE_COLOR_FONT_RED "[Assert]" CONSOLE_COLOR_END "[%s/%s](%d) " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
exit(-1);\
};

struct STRCMP_Compare
{
    bool operator()(const char * a, const char * b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

#include "_info.h"

#endif /* _DEFINES_H_ */
