#include "prompt.h"
#include "buffers.h"
#include "esc_def.h"
#include "input.h"
#include "algorithm.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// tak. pisze to gdy zaraz beda swieta :)
static void print_prompt(char* const user_name, char* const host_name,
    char* const working_dir)
{
    printf(ESC_SGR_FGR_COLOR_RGB(220, 20, 20));
    printf("<");

    printf(ESC_SGR_BGR_COLOR_RGB(30, 0, 100));
    printf(ESC_SGR_FGR_COLOR_RGB(220, 220, 120));
    printf("%s", user_name);

    printf(ESC_SGR_FGR_COLOR_RGB(220, 20, 20));
    printf("@");

    printf(ESC_SGR_FGR_COLOR_RGB(120, 220, 220));
    printf("%s", host_name);

    printf(ESC_SGR_CLEAR);
    printf(" ");

    printf(ESC_SGR_UNDERLINE);
    printf(ESC_SGR_FGR_COLOR_RGB(30, 130, 40));
    printf("%s", working_dir);
    printf(ESC_SGR_UNDERLINE_STOP);

    printf(ESC_SGR_FGR_COLOR_RGB(220, 20, 20));
    printf(">");

    printf(ESC_SGR_FGR_COLOR_RGB(220, 220, 120));
    printf(ESC_SGR_BLINK_SLOW);
    printf("$");

    printf(ESC_SGR_CLEAR);
    printf(" ");

    fflush(stdout);
}

static struct char_buff read_cwd(void)
{
    struct char_buff buff = char_buff_create();
    char_buff_alloc(&buff, 128);

    do {
        char* ptr = getcwd(buff.data, buff.capacity);
        if (ptr) {
            break;
        }

        char_buff_alloc(&buff, 2 * buff.capacity);
    } while (true);

    char* null_position = find_char(buff.data, buff.data + buff.capacity, '\0');
    if (null_position < buff.data + buff.capacity) {
        buff.size = null_position + 1 - buff.data;
    } else {
        buff.size = null_position - buff.data;
        char_buff_add(&buff, '\0');
    }

    return buff;
}

static struct char_buff read_host_name(void)
{
    struct char_buff buff = char_buff_create();
    char_buff_alloc(&buff, 32);

    do {
        int val = gethostname(buff.data, buff.capacity - 1);
        buff.data[buff.capacity - 1] = '\0';
        if (val == 0) {
            break;
        }

        char_buff_alloc(&buff, 2 * buff.capacity);
    } while (true);

    char* null_position = find_char(buff.data, buff.data + buff.capacity, '\0');
    if (null_position < buff.data + buff.capacity) {
        buff.size = null_position + 1 - buff.data;
    } else {
        buff.size = null_position - buff.data;
        char_buff_add(&buff, '\0');
    }

    return buff;
}

static struct char_buff read_input()
{
    struct char_buff buff = char_buff_create();
    char_buff_alloc(&buff, 128);

    if (!wait_input()) {
        char_buff_add(&buff, '\0');
        return buff;
    }

    while (true) {

        char c = input();
        if (c == '\n' || c == '\0') {
            char_buff_add(&buff, '\0');
            return buff;
        }
        if (c == (char)EOF) {
            if (buff.size == 0) {
                char_buff_add(&buff, (char)EOF);
                return buff;
            } else {
                // aby nacisniecie ctrld jezeli cos jest napisane
                // nie powodowalo zamkniecia stdina
                // dokladnie tak jak w bashu
                clearerr(stdin);
                continue;
            }
        }

        char_buff_add(&buff, c);
    }
}

struct char_buff prompt()
{
    struct char_buff cwd = read_cwd();
    struct char_buff host_name = read_host_name();
    char* user = getenv("USER"); // pole statyczne

    print_prompt(user, host_name.data, cwd.data);
    struct char_buff input = read_input();

    char_buff_free(&cwd);
    char_buff_free(&host_name);

    return input;
}