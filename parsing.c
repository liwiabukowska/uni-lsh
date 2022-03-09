#include "parsing.h"

#include "algorithm.h"
#include "buffers.h"

#include <ctype.h>
#include <stdbool.h>

// do debugu
#include <stdio.h>

void print_argc_argv(uint32_t argc, char** argv)
{
    printf("argc: %u\n", argc);
    for (uint32_t i = 0; i < argc; ++i) {
        printf("arg %u: \"%s\"\n", i, argv[i]);
    }
}

struct ptr_buff parse_job_argv(char* begin, char* end)
{
    struct ptr_buff argv = ptr_buff_create();
    ptr_buff_alloc(&argv, 16);

    bool ready = true;
    bool inside_quote = false;
    for (char* it = begin; it < end; ++it) {
        if (inside_quote || isgraph(*it)) {
            if (*it == '\"') {
                inside_quote = !inside_quote;
                *it = '\0';
                ready = true;
            } else if (ready) {
                ptr_buff_add(&argv, it);
                ready = false;
            }
        } else {
            *it = '\0';
            ready = true;
        }
    }
    ptr_buff_add(&argv, NULL);

    return argv;
}

struct job_info_buff parse(char* begin, char* end)
{

    struct job_info_buff jobs = job_info_buff_create();
    job_info_buff_alloc(&jobs, 4); // FIXME jezeli automatycznie alokuje ale
    // dla wiekszych niz 0 to dla 0 tez moglby choc poczatkowo byl to zwykly bufor

    char* job_begin = begin;
    while (job_begin < end) {
        char* ampersand = find_char(job_begin, end, '&');
        char* semicolon = find_char(job_begin, end, ';');
        char* job_end = ampersand < semicolon ? ampersand : semicolon;

        bool is_waiting = true;
        if (job_end != end) {
            if (*job_end == '&') {
                is_waiting = false;
            }
            *job_end = '\0';
        }

        struct char_buff job_buffer = char_buff_copy_span(job_begin, job_end + 1);
        struct char_buff to_parse = char_buff_copy_span(job_begin, job_end + 1);
        struct command_buff commands = command_buff_create();
        command_buff_alloc(&commands, 2);

        // modyfikuj kopie
        char* job_parse_begin = to_parse.data;
        char* job_parse_end = to_parse.data + to_parse.size;

        char* command_begin = job_parse_begin;

        bool is_file_stdin = false;
        bool is_file_stdout = false;
        bool is_file_stderr = false;
        bool is_pipe_found = false;

        while (command_begin < job_parse_end) {

            // znaki musza byc rozpoznawane z odpowiednim priorytetem dlatego niepokolei

            char* command_end = find_char(command_begin, job_parse_end, '|');
            if (command_end != job_parse_end) {
                *command_end = '\0';
                is_pipe_found = true;
            }

            // musi go zarezerwowac zanim zobaczy go >
            char* err_found = find_substr(command_begin, command_end, "2>");
            if (err_found != command_end) {
                err_found[0] = '\0';
                err_found[1] = '\0';
                is_file_stderr = true;
            }

            char* in_found = find_char(command_begin, command_end, '<');
            char* out_found = find_char(command_begin, command_end, '>');
            if (in_found != command_end) {
                *in_found = '\0';
                is_file_stdin = true;
            }
            if (out_found != command_end) {
                *out_found = '\0';
                is_file_stdout = true;
            }

            // znaczenie i priorytety operatorow zostaly rozpoznane
            // czas zparsowac i zapisac komendy miedzy nimi

            struct ptr_buff argv = parse_job_argv(
                command_begin,
                find_char(command_begin, command_end, '\0'));

            struct ptr_buff stdin_buff = ptr_buff_create();
            if (is_file_stdin) {
                stdin_buff = parse_job_argv(
                    in_found + 1,
                    find_char(in_found + 1, command_end, '\0'));
                if (stdin_buff.size == 0) {
                    is_file_stdin = false;
                }
            }
            struct ptr_buff stdout_buff = ptr_buff_create();
            if (is_file_stdout) {
                stdout_buff = parse_job_argv(
                    out_found + 1,
                    find_char(out_found + 1, command_end, '\0'));
                if (stdout_buff.size == 0) {
                    is_file_stdout = false;
                }
            }
            struct ptr_buff stderr_buff = ptr_buff_create();
            if (is_file_stderr) {
                stderr_buff = parse_job_argv(
                    err_found + 2,
                    find_char(err_found + 2, command_end, '\0'));
                if (stderr_buff.size == 0) {
                    is_file_stderr = false;
                }
            }

#ifdef DEBUG
            printf("Debug: parsowanie komendy\n");
            print_argc_argv(argv.size, (char**)argv.data);
            if (is_file_stdin) {
                printf("Debug: stdin do \"%s\"\n", (char*)stdin_buff.data[0]);
            }
            if (is_file_stdout) {
                printf("Debug: stdout do \"%s\"\n", (char*)stdout_buff.data[0]);
            }
            if (is_file_stderr) {
                printf("Debug: stderr do \"%s\"\n", (char*)stderr_buff.data[0]);
            }
            if (is_pipe_found) {
                printf("Debug: jest pipe\n");
            }
            if (is_waiting) {
                printf("Debug: jest czekajaca\n");
            }
#endif

            char* stdin_file = NULL;
            if (is_file_stdin) {
                stdin_file = stdin_buff.data[0];
            }
            char* stdout_file = NULL;
            if (is_file_stdout) {
                stdout_file = stdout_buff.data[0];
            }
            char* stderr_file = NULL;
            if (is_file_stderr) {
                stderr_file = stderr_buff.data[0];
            }

            struct char_buff command_buffer = char_buff_copy_span(command_begin, command_end);
            struct command_info command = command_info_create(
                &command_buffer,
                argv.size - 1, &argv,
                stdin_file,
                stdout_file,
                stderr_file,
                is_pipe_found); // TODO dokoncz

            command_buff_add(&commands, &command);

            // ptr_buff_free(&argv); // command przejal posiadanie
            ptr_buff_free(&stdin_buff);
            ptr_buff_free(&stdout_buff);
            ptr_buff_free(&stderr_buff);
            is_pipe_found = false;
            is_file_stdin = false;
            is_file_stdout = false;
            is_file_stderr = false;

            command_begin = command_end + 1;
        }

        struct job_info job = job_info_create(
            &job_buffer,
            &commands,
            is_waiting);

        job_info_buff_add(&jobs, &job);

        job_begin = job_end + 1;
    }

    return jobs;
}