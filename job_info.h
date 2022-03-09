#pragma once

#include "buffers.h"

#include <stdbool.h>
#include <stdint.h>

struct command_info {
    struct char_buff parsed_buffer;
    struct ptr_buff argv;
    char* stdin_file; // lub null jezeli nie ma
    char* stdout_file; //..
    char* stderr_file; //..
    uint32_t argc; // w zasadzie teraz juz jest size w argv no ale tak to juz jest jak sie robi refactor kontenerow
    bool is_piped;
};

struct command_info command_info_create(
    struct char_buff* parsed_buffer,
    uint32_t argc,
    struct ptr_buff* argv, // otrzymuje na wlasnosc
    char* stdin_file,
    char* stdout_file,
    char* stderr_file,
    bool is_piped);

void command_info_free(struct command_info* job);

struct command_buff {
    size_t capacity;
    size_t size;
    struct command_info* data;
};

struct command_buff command_buff_create();

bool command_buff_alloc(struct command_buff* buff, size_t new_size);

bool command_buff_add(struct command_buff* buff, struct command_info* command);

void command_buff_free(struct command_buff* buff);

struct job_info {
    struct char_buff orginal_buffer;
    struct command_buff commands;
    bool is_waiting;
};

struct job_info job_info_create(
    struct char_buff* orginal_buffer,
    struct command_buff* commands,
    bool is_waiting);

void job_info_free(struct job_info* job);

// rowniez do przenoszenia ownershipa zasobow joba gdy jeden jest pusty
void job_info_swap(struct job_info* a, struct job_info* b);

struct job_info_buff {
    size_t capacity;
    size_t size;
    struct job_info* data;
};

struct job_info_buff job_info_buff_create(void);

bool job_info_buff_alloc(struct job_info_buff* buff, size_t new_size);

bool job_info_buff_add(struct job_info_buff* buff, struct job_info* command);

void job_info_buff_free(struct job_info_buff* buff);