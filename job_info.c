#include "job_info.h"

#include "lsh_exit.h"

#include <stdlib.h>

struct command_info command_info_create(
    struct char_buff* parsed_buffer,
    uint32_t argc,
    struct ptr_buff* argv, // otrzymuje na wlasnosc
    char* stdin_file,
    char* stdout_file,
    char* stderr_file,
    bool is_piped)
{
    struct command_info job = {
        *parsed_buffer,
        *argv,
        stdin_file,
        stdout_file,
        stderr_file,
        argc,
        is_piped
    };

    return job;
}

void command_info_free(struct command_info* job)
{
    ptr_buff_free(&job->argv);
    char_buff_free(&job->parsed_buffer);
}

struct command_buff command_buff_create()
{
    struct command_buff buff = {
        0,
        0,
        NULL
    };

    return buff;
}

bool command_buff_alloc(struct command_buff* buff, size_t new_capacity)
{
    struct command_info* ptr = realloc(buff->data,
        new_capacity * sizeof(struct command_info));

    if (!ptr) {
        exit_with_message(1, "malloc error!!!\n");
        return false;
    }

    buff->capacity = new_capacity;
    buff->data = ptr;
    return true;
}

bool command_buff_add(struct command_buff* buff, struct command_info* command)
{
    if (buff->size == buff->capacity) {
        if (!command_buff_alloc(buff, 2 * buff->capacity)) {
            return false;
        }
    }

    buff->data[buff->size] = *command;
    ++buff->size;
    return true;
}

void command_buff_free(struct command_buff* buff)
{
    for (uint32_t i = 0; i < buff->size; ++i) {
        command_info_free(&buff->data[i]);
    }

    free(buff->data);
    buff->data = NULL;
    buff->capacity = 0;
    buff->size = 0;
}

struct job_info job_info_create(
    struct char_buff* orginal_buffer,
    struct command_buff* commands,
    bool is_waiting)
{
    struct job_info job = {
        *orginal_buffer,
        *commands,
        is_waiting
    };

    return job;
}

void job_info_swap(struct job_info* a, struct job_info* b)
{
    struct job_info tmp = *a;
    *a = *b;
    *b = tmp;
}

void job_info_free(struct job_info* job)
{
    char_buff_free(&job->orginal_buffer);
    command_buff_free(&job->commands);
}

struct job_info_buff job_info_buff_create(void)
{
    struct job_info_buff buff = {
        0,
        0,
        NULL
    };

    return buff;
}

bool job_info_buff_alloc(struct job_info_buff* buff, size_t new_capacity)
{
    struct job_info* ptr = realloc(buff->data,
        new_capacity * sizeof(struct job_info));

    if (!ptr) {
        exit_with_message(1, "malloc error!!!\n");
        return false;
    }

    buff->capacity = new_capacity;
    buff->data = ptr;
    return true;
}

bool job_info_buff_add(struct job_info_buff* buff, struct job_info* command)
{
    if (buff->size == buff->capacity) {
        if (!job_info_buff_alloc(buff, 2 * buff->capacity)) {
            return false;
        }
    }

    buff->data[buff->size] = *command;
    ++buff->size;
    return true;
}

void job_info_buff_free(struct job_info_buff* buff)
{
    for (uint32_t i = 0; i < buff->size; ++i) {
        job_info_free(&buff->data[i]);
    }

    free(buff->data);
    buff->data = NULL;
    buff->capacity = 0;
    buff->size = 0;
}