#include "lsh_exit.h"

#include "job_management.h"

#include <unistd.h>
#include <stdio.h>

void exit_shell(int status)
{
    printf("\n");
    
    // wszystkie niezwaitowane joby zostana dziecmi pid1 ktory je zreapuje
    // ale moge byc nadgorliwy i zamykac chociaz te ktore zakonczyly sie przez zakonczeniem lsh
    collect_all_child_processes();

    _exit(status);
}

void exit_with_message(int status, char* message)
{
    printf("%s", message);
    exit_shell(status);
}