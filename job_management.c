#include "job_management.h"

#include "buffers.h"
#include "input.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void set_console_fg_group(struct running_job* running)
{
#ifdef DEBUG
    printf("set console group: %p", running);
#endif

    pid_t pid = running ? running->pids.data[0] : getpid();

    if (tcsetpgrp(0, pid)) {
        printf("set_console_fg_group: blad");
        return;
    }

#ifdef DEBUG
    printf("--> %u\n", pid);
#endif
}

void log_status(uint64_t pid, int status)
{
    printf("wait_all_pids: waitpid: %lu<%i> ", pid, status);
    if (WIFSIGNALED(status)) {
        printf("na sygnale %i", WTERMSIG(status));
    }
    printf("\n");
}

bool wait_all_pids(struct running_job* running, int options)
{
#ifdef DEBUG
    printf("Debug: wait_all_pids\n");
#endif

    bool all_exited = true;
    uint32_t i = 0;
    while (i < running->pids.size) {
        uint64_t pid = running->pids.data[i];
        int status = 0;
        waitpid(pid, &status, options);

#ifdef DEBUG
        log_status(pid, status);
#endif
        // ctrl c nie daje exited a signalled
        if (WIFEXITED(status)) {
            uint64_buff_erase(&running->pids, i);

#ifdef DEBUG
            printf("wait_all_pids: %lu zakonczono\n", running->pids.data[i]);
#endif
        } else {
            all_exited = false;
            ++i;
        }
    }

    return all_exited;
}

void send_signal_to_job(struct running_job* running, int signal)
{
    for (uint32_t i = 0; i < running->pids.size; ++i) {
        int val = kill((pid_t)running->pids.data[i], signal);
        if (val) {
#ifdef DEBUG
            printf("running_job_send_signal_to_all:"
                   "kill: blad %i podczas wysylania sygnalu do procesu %lu\n",
                val, running->pids.data[i]);
#endif
        }
    }
}

void interrupt_job(struct running_job* running)
{
#ifdef DEBUG
    printf("Debug: interrupt_job\n");
#endif
    send_signal_to_job(running, SIGINT);
    // wait_all_pids(running, 0); // sigint moze zostac przechwyocny
}

void stop_job(struct running_job* running)
{
    send_signal_to_job(running, SIGSTOP);
    wait_all_pids(running, WSTOPPED);
}

void kill_job(struct running_job* running)
{
    send_signal_to_job(running, SIGKILL);
    wait_all_pids(running, 0);
}

bool wait_fg_job(struct running_job* running)
{
#ifdef DEBUG
    printf("Debug: foreground_job\n");
#endif

    // job moze wyjsc z tej funkcji poprzez zastopowanie
    bool exited = wait_all_pids(running, WSTOPPED);
    set_console_fg_group(NULL);

    return exited;
}

void foreground_job(struct running_job* running)
{
#ifdef DEBUG
    printf("Debug: foreground_job\n");
#endif
    set_console_fg_group(running);
    send_signal_to_job(running, SIGCONT);
}

void background_job(struct running_job* running)
{
#ifdef DEBUG
    printf("Debug: background_job\n");
#endif
    send_signal_to_job(running, SIGCONT);
    set_console_fg_group(NULL);
}

void collect_all_child_processes(void)
{
    while (true) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid == -1) {
            // nie ma dzieci

            break;
        } else if (pid == 0) {
            // wszystkie dzieci jeszcze sa aktywne
            printf("niektore dzieci jeszcze zyja!\n");

            break;
        } else {
            if (pid > 0) {
                printf("zamykam zakonczony proces: %u[%i] ", pid, status);
            }
        }
    }
    printf("\n");
}