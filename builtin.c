#include "builtin.h"

#include "active_job.h"
#include "algorithm.h"
#include "buffers.h"
#include "job_info.h"
#include "job_management.h"
#include "lsh_exit.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// okazuje sie ze w procfs pliki nie maja size i lseek nie dziala :|
void print_state(uint64_t pid)
{
    char fname[32] = { 0 };
    sprintf(fname, "/proc/%lu/status", pid);

    FILE* fd = fopen(fname, "r");
    if (!fd) {
        printf("read_state: blad open");
        goto free_resources;
    }

    struct char_buff content = char_buff_create();
    char_buff_alloc(&content, 1024);

    while (true) {
        char c = fgetc(fd);
        if (c == EOF) {
            char_buff_add(&content, '\0');
            break;
        }

        char_buff_add(&content, c);
    }

    char* end = find_char(content.data,
        content.data + content.size, '\0');

    char* state = find_substr(content.data, end,
        "State:");
    if (state == end) {
        printf("read_state: blad szukania state");
        goto free_resources;
    }

    char* left = find_char(state, end, '(');
    char* right = find_char(left, end, ')');
    if (left == end) {
        printf("read_state: blad parsowania");
        goto free_resources;
    }
    // right nie trzeba sprawdzac
    // nie przeszkadza jak obetnie stan bez )

    *left = *right = '\0';
    printf("%s", left + 1);

free_resources:
    fclose(fd);
    char_buff_free(&content);
}

void builtin_help(int argc, char** argv)
{
    printf("help: lsh - prosty?? shell\n");
    printf("    komendy wbudowane:\n");
    printf("    exit      - wychodzi z shella\n");
    printf("    ctrl+d    - wychodzi z shella\n");
    printf("    ctrl+c    - zamyka job w terminalu\n");
    printf("    cd        - zmienia folder roboczy\n");
    printf("    jobs      - pokazuje status jobow i procesow w tle\n");
    printf("    fg        - czeka na podany %%job\n");
    printf("    bg        - uruchamia zatrzymany %%job w tle\n");
    printf("    ctrl+z    - zatrzymuje aktualny job\n");
    printf("    stop      - ctrl+z z %%job\n");
}

void builtin_exit(int argc, char** argv)
{
    printf("exit");
    exit_shell(0);
}

void builtin_cd(int argc, char** argv)
{
    if (argc != 2) {
        exit_with_message(1, "niepoprawny format cd -- zla ilosc argumentow");
        return;
    }

    if (chdir(argv[1])) {
        printf("blad podczas zmiany folderu: \"%s\"\n", argv[1]);
    }
}

void builtin_jobs(int argc, char** argv)
{
    printf("jobs: %lu jobow jest aktywnych\n", g_running_jobs.size);

    uint32_t job_index = 0;
    while (job_index < g_running_jobs.size) {
        struct running_job* current_job = &g_running_jobs.data[job_index];

        printf("%u: \"%s\"\n", job_index + 1,
            current_job->job.orginal_buffer.data);

        printf("    pidy: ");
        bool all_exited = true;
        uint32_t pid_index = 0;
        while (pid_index < current_job->pids.size) {

            int status = 0;
            int ret = waitpid(current_job->pids.data[pid_index], &status, WNOHANG);
            if (ret == -1) {
                // job umarl i zostal zebrany

                uint64_buff_erase(&current_job->pids, pid_index);
                printf("\njobs: waitpid %lu=-1 -- proces niespodziewanie umarl\n", current_job->pids.data[pid_index]);
            } else if (ret == 0) {
                // pid nie zakonczyl sie jeszcze

                printf("%lu<", current_job->pids.data[pid_index]);
                print_state(current_job->pids.data[pid_index]);
                printf("> ");

                all_exited = false;
                ++pid_index;
            } else {
                // job zostal zakonczony

                printf("%lu[%i] ", current_job->pids.data[pid_index], status);
                uint64_buff_erase(&current_job->pids, pid_index);
            }
        }

        if (all_exited) {
            running_job_buff_erase(&g_running_jobs, job_index);
            printf("    zakonczono\n");
        } else {
            ++job_index;
            printf("\n");
        }
    }
}

static bool parse_job_number(int argc, char** argv, uint32_t* r_val)
{
    if (argc <= 1) {
        return false;
    }

    char* it = argv[1];
    while (*it != '\0') {
        if (*it == '%') {
            break;
        }

        ++it;
    }

    if (it[0] == '%' && isdigit(it[1])) {
        *r_val = atoi(it + 1);
        return true;
    }

    return false;
}

void builtin_fg(int argc, char** argv)
{
    uint32_t job_num = 0;
    if (!parse_job_number(argc, argv, &job_num)) {
        printf("fg: niepoprawny format numeru joba\n");
        return;
    }

    if (job_num < 1 && job_num > g_running_jobs.size) {
        printf("fg: nie ma takiego joba\n");
        return;
    }

    struct running_job* running = &g_running_jobs.data[job_num - 1];
    foreground_job(running);
    if (wait_fg_job(running)) {
        // musze wziac pod uwage ze moze wyjsc bez
        // zakonczenia joba np przez stop
        running_job_free(running);
        running_job_buff_erase(&g_running_jobs, job_num - 1);
    }
}

void builtin_bg(int argc, char** argv)
{
    uint32_t job_num = 0;
    if (!parse_job_number(argc, argv, &job_num)) {
        printf("bg: niepoprawny format numeru joba\n");
        return;
    }

    if (job_num < 1 && job_num > g_running_jobs.size) {
        printf("bg: nie ma takiego joba\n");
        return;
    }

    background_job(&g_running_jobs.data[job_num - 1]);
}

void builtin_stop(int argc, char** argv)
{
    uint32_t job_num = 0;
    if (!parse_job_number(argc, argv, &job_num)) {
        printf("stop: niepoprawny format numeru joba\n");
        return;
    }

    if (job_num < 1 && job_num > g_running_jobs.size) {
        printf("stop: nie ma takiego joba\n");
        return;
    }

    stop_job(&g_running_jobs.data[job_num - 1]);
}

// zwraca false jezeli nie byla to kokmenda wbudowana
bool builtin(int argc, char** argv)
{
    if (argc == 0) {
        // po prostu ignoruj pusta komende
        // printf("pusta komenda\n");
        return true;
    }

    if (0 == strcmp(argv[0], "exit")) {
        builtin_exit(argc, argv);
        return true;
    } else if (0 == strcmp(argv[0], "help")) {
        builtin_help(argc, argv);
        return true;
    } else if (0 == strcmp(argv[0], "cd")) {
        builtin_cd(argc, argv);
        return true;
    } else if (0 == strcmp(argv[0], "jobs")) {
        builtin_jobs(argc, argv);
        return true;
    } else if (0 == strcmp(argv[0], "fg")) {
        builtin_fg(argc, argv);
        return true;
    } else if (0 == strcmp(argv[0], "bg")) {
        builtin_bg(argc, argv);
        return true;
    } else if (0 == strcmp(argv[0], "stop")) {
        builtin_stop(argc, argv);
        return true;
    }

    return false;
}