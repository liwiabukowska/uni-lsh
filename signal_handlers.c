#include "signal_handlers.h"

#include "job_management.h"

#include <signal.h>
#include <stdio.h>


void sigtstp_handler(int signal)
{
    printf("\n");
#ifdef DEBUG
    printf("Debug: SIGTSTP\n");
#endif
}

void sigint_handler(int signal)
{
    printf("\n");
#ifdef DEBUG
    printf("Debug: SIGINT\n");
#endif
}

void signal_init(void)
{
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGINT, sigint_handler);

    // ignoruj sigtou aby program nie zostawal zatrzymywany
    // podczas ustawiania spowrotem na siebie
    // kontroli nad konsola kontrolujaca
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
}

void signal_reset(void)
{
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}