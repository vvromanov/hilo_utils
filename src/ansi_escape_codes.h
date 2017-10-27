#pragma once

#define ANSI_ATTR_RESET           "\x1B[0m"
#define ANSI_BOLD_GRAY            "\x1B[1;30m"
#define ANSI_GRAY                 "\x1B[0;37m"
#define ANSI_BOLD_RED             "\x1B[1;31m"
#define ANSI_BOLD_GREEN           "\x1B[1;32m"
#define ANSI_BOLD_YELLOW          "\x1B[1;33m"
#define ANSI_BOLD_BLUE            "\x1B[1;34m"
#define ANSI_BLUE                 "\x1B[0;34m"
#define ANSI_BRIGHT_BLUE          "\x1B[34;1m"
#define ANSI_BOLD_CYAN            "\x1B[1;36m"
#define ANSI_BOLD_YELLOW_ON_RED   "\x1B[1;33;41m"

#define VT_CLEAR_SCREEN   "\x1B[2J"
#define VT_CLEAR_LINE_END "\x1B[K"
/*
#define VT_CURSOR_HOME "\033[H"
#define VT_CURSOR_SAVE "\0337"
#define VT_CURSOR_RESTORE "\0338"
*/
