#include "input.h"

#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/poll.h>

// musze przestac czekac na getchara to ctrlc

// pomysl z
// https://forums.codeguru.com/showthread.php?343829-break-blocked-getchar-call
// stworzyc pipa i czeka na input z niego lub stdina
// a do pipa mozna pisac zeby odblokowac czekanie
// i przepisalem na polla zamiast selecta

int unblock = 0; // (write-end of the pipe)
int block_check = 0; // (read-end of the pipe)

char input()
{
    return getchar();
}

void cancel_input()
{
    if (unblock) {
        write(unblock, "", 1); // trzeba zapisac cokolwiek do pipa
        close(unblock);
        unblock = 0;
    }
}

bool wait_input()
{
    if (!unblock) {
        // jezeli nie ma pipow to stworz

        int unblock_pipe[2];
        if (pipe(unblock_pipe))
            return false;
        unblock = unblock_pipe[1];
        block_check = unblock_pipe[0];
    }
    struct pollfd polls[2] = {
        { 0, POLLIN, 0 },
        { block_check, POLLIN, 0 }
    };

    // zaden nie zostal zmodyfikowany
    int ret = poll(polls, 2, -1);
    if (ret == -1) {
#ifdef Debug
        printf("DEBUG: wait_input: poll zwraca blad\n");
#endif

        return false;
    } else if (ret == 0) {
#ifdef Debug
        printf("DEBUG: wait_input: poll zwraca timeout\n");
#endif
        return false;
    } else if (ret != 0) {

        if (polls[0].revents) {
            // jezeli stdin zostal zmodyfikowany
            return true;
        } else if (polls[1].revents) {
            // jezeli pipe blokowania zostal zmodyfikwany
            if (block_check) {
                close(block_check);
            }

            block_check = 0;
            return false;
        } else {
#ifdef Debug
            printf("DEBUG: wait_input: poll nas oklamuje\n");
#endif
            return false;
        }

    }

    return false; // nie zwroci nic innego
}