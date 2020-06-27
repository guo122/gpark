

#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <iostream>

#define CONSOLE_COLOR_END               "\033[0m"

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

#define CONSOLE_COLOR_WHITE_BLACK       "\033[30;47m"

#define REPOS_PATH_FOLDER               "/_gpark"
#define REPOS_PATH_INDEX                "/_gpark/index"
#define REPOS_PATH_DB                   "/_gpark/db"

#define DB_OFFSET_LENGTH    8

#define SHA_CHAR_LENGTH     20

#define GAssert(expression, format, ...) \
if (!(expression)) { \
printf(CONSOLE_COLOR_FONT_RED "[Assert]" CONSOLE_COLOR_END "[%s/%s](%d) " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
};

#endif /* _DEFINES_H_ */
