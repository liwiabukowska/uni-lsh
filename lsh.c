/*
dodatkowe featuresy
pelna dowolnosc dlugosci danych do wpisania,
    ilosci argumentow,
    nic nie jest na sztywno
help
*/

/*
TODO:
 - todo zamykanie jobow na zamykanie shella
*/

#include "active_job.h"
#include "algorithm.h"
#include "buffers.h"
#include "builtin.h"
#include "job_info.h"
#include "job_management.h"
#include "lsh_exit.h"
#include "parsing.h"
#include "prompt.h"
#include "signal_handlers.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// jezeli fd -1 to pomija jego podmiane
pid_t fork_thread_fd(uint32_t argc, char** argv,
    int in_fd, int out_fd, int err_fd)
{
    pid_t pid = fork();
    if (pid < 0) {
        exit_with_message(1, "fork");
    } else if (pid == 0) {

        // zostawiam selfpipa
        // program ma otworzony fd pisania do siebie odziedziczony z parrenta
        // jezeli wchodzi do niego pipe
        // ale interfejs tej funkcji nie pozwala na podanie ktory to fd
        // tak wiec pozostawiam go wiszacego
        // podczas zamykania procesu childa i tak sie zamknie

        if (in_fd != -1) {
            close(0);
            dup2(in_fd, 0);
            close(in_fd);
        }
        if (out_fd != -1) {
            close(1);
            dup2(out_fd, 1);
            close(out_fd);
        }
        if (err_fd != -1) {
            close(2);
            dup2(err_fd, 2);
            close(err_fd);
        }

        // trzeba zresetowac ustawione sygnaly aby
        // dziecko nie ignorowalo np sigttou
        signal_reset();

        if (execvp(argv[0], argv) < 0) {
            exit_with_message(1, "execvp: nie mozna wywolac komendy");
        }
    }

    return pid;
}

// running job przechwytuje ownership joba
// zwraca false jesli blad
bool execute(struct job_info* job, struct running_job* r_running)
{
#ifdef DEBUG
    printf("Debug: (wykonywanie komendy) %s\n", job->orginal_buffer.data);
#endif
    { // zmiana ownershipa danych
        struct uint64_buff pids = uint64_buff_create();
        uint64_buff_alloc(&pids, 8);

        struct job_info job_moved = {};
        job_info_swap(&job_moved, job); // podmiana na pusty

        *r_running = running_job_create(&job_moved, &pids);

        job = &job_moved;
    }

    int last_pipe = -1;
    pid_t job_pgid = 0;
    for (uint32_t i = 0; i < job->commands.size; ++i) {

        int in_fd = -1;
        int out_fd = -1;
        int err_fd = -1;
        if (builtin(job->commands.data[i].argc, (char**)job->commands.data[i].argv.data)) {
            // byla wbudowana

            if (job->commands.data[i].is_piped) {
                printf("\"%s\" : nie mozesz | do wbudowanej komendy!\n",
                    (char*)job->commands.data[i].argv.data[0]);
                return false;
            }
            if (job->commands.data[i].stdin_file) {
                printf("\"%s\" : nie mozesz < wbudowanej komendy!\n",
                    (char*)job->commands.data[i].argv.data[0]);
                return false;
            }
            if (job->commands.data[i].stdout_file) {
                printf("\"%s\" : nie mozesz > wbudowanej komendy!\n",
                    (char*)job->commands.data[i].argv.data[0]);
                return false;
            }
            if (job->commands.data[i].stderr_file) {
                printf("\"%s\" : nie mozesz 2> wbudowanej komendy!\n",
                    (char*)job->commands.data[i].argv.data[0]);
                return false;
            }

        } else {
            // byla zewnetrzna

            if (last_pipe != -1) {
                in_fd = last_pipe;
                last_pipe = -1;
            } else {
                if (job->commands.data[i].stdin_file) {
                    in_fd = open(job->commands.data[i].stdin_file, O_RDONLY, 0644);
                    if (in_fd == -1) {
                        printf("open: nie mozna otworzyc pliku \"%s\" jako stdin\n",
                            job->commands.data[i].stdin_file);
                        return false;
                    }
                }
            }
            if (job->commands.data[i].is_piped) {
                int pipe_fd[2];
                if (!pipe(pipe_fd)) {
                    out_fd = pipe_fd[1];
                    last_pipe = pipe_fd[0];
                } else {
                    printf("pipe: nie mozna otworzyc\n");
                    return false;
                }
            } else {
                if (job->commands.data[i].stdout_file) {
                    out_fd = open(job->commands.data[i].stdout_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                    if (out_fd == -1) {
                        printf("open: nie mozna otworzyc pliku \"%s\" jako stdout\n",
                            job->commands.data[i].stdout_file);
                        return false;
                    }
                }
            }
            if (job->commands.data[i].stderr_file) {
                err_fd = open(job->commands.data[i].stderr_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                if (err_fd == -1) {
                    printf("open: nie mozna otworzyc pliku \"%s\" jako stderr\n",
                        job->commands.data[i].stderr_file);
                }
            }

            pid_t child_pid = fork_thread_fd(
                job->commands.data[i].argc,
                (char**)job->commands.data[i].argv.data,
                in_fd, out_fd, err_fd);
            if (job_pgid == 0) {
                job_pgid = child_pid;
            }

            if (setpgid(child_pid, job_pgid)) {
                return false;
            }

            uint64_buff_add(&r_running->pids, child_pid);
        }

        if (in_fd != -1) {
            close(in_fd);
        }
        if (out_fd != -1) {
            close(out_fd);
        }
        if (err_fd != -1) {
            close(err_fd);
        }
    }

    return true;
}

struct running_job_buff g_running_jobs;

int main()
{
    g_running_jobs = running_job_buff_create();
    running_job_buff_alloc(&g_running_jobs, 4);

    signal_init();

    while (true) {

        struct char_buff input = prompt();
        if (input.data[0] == (char)EOF) {
            char_buff_free(&input);
            break;
        }

        struct job_info_buff jobs = parse(input.data, input.data + input.size);
        for (uint32_t i = 0; i < jobs.size; ++i) {

            struct running_job new_job = {};
            if (!execute(&jobs.data[i], &new_job)) {
                printf("lsh: wystapily bledy przy uruchamianiu joba -- "
                       "    nastapi kill wszystkich jego procesow\n");

                kill_job(&new_job);
                running_job_free(&new_job);
                continue;
            }
            if (!new_job.pids.size) {
                // wbudowana
                running_job_free(&new_job);
                continue;
            }

            running_job_buff_add(&g_running_jobs, &new_job);
            struct running_job* running = &g_running_jobs.data[g_running_jobs.size - 1];

            if (running->job.is_waiting) {
                foreground_job(running);
                if (wait_fg_job(running)) {
                    running_job_free(running);
                    running_job_buff_erase(&g_running_jobs, g_running_jobs.size - 1);
                }
            }
        }

        job_info_buff_free(&jobs);
        char_buff_free(&input);
    }

    running_job_buff_free(&g_running_jobs);

    exit_shell(0);
}