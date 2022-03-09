#include "active_job.h"

#include "buffers.h"
#include "job_info.h"
#include "lsh_exit.h"

#include <stdlib.h>

struct running_job running_job_create(struct job_info* job, struct uint64_buff* pids)
{
    struct running_job tmp = {
        *job,
        *pids
    };

    return tmp;
}

void running_job_free(struct running_job* running)
{
    job_info_free(&running->job);
    uint64_buff_free(&running->pids);
}

struct running_job_buff running_job_buff_create()
{
    struct running_job_buff buff = {
        0, 0,
        NULL
    };

    return buff;
}

bool running_job_buff_alloc(struct running_job_buff* buff, size_t new_capacity)
{
    struct running_job* ptr = realloc(buff->data,
        new_capacity * sizeof(struct running_job));
    if (!ptr) {
        exit_with_message(1, "malloc error!!!\n");
        return false;
    }

    buff->capacity = new_capacity;
    buff->data = ptr;
    return true;
}

bool running_job_buff_add(struct running_job_buff* buff, struct running_job* ptr)
{
    if (buff->size == buff->capacity) {
        if (!running_job_buff_alloc(buff, 2 * buff->capacity)) {
            return false;
        }
    }

    buff->data[buff->size] = *ptr;
    ++buff->size;
    return true;
}

void running_job_buff_free(struct running_job_buff* buff)
{
    free(buff->data);
    buff->data = NULL;
    buff->capacity = 0;
    buff->size = 0;
}

void running_job_buff_erase(struct running_job_buff* buff, uint32_t index)
{ // TODO stabilne usuwanie z kopiowaniem zachowa kolejnosc wywolywania jobow

    buff->data[index] = buff->data[buff->size - 1];
    --buff->size;
}