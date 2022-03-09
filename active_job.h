#pragma once

#include "buffers.h"
#include "job_info.h"

// uruchomionych nie musi byc tyle co commandow w jobsie
// bo wbudowane oraz bledy podczas executowania
struct running_job {
    struct job_info job;
    struct uint64_buff pids; // uruchmione procesy
};

struct running_job running_job_create(struct job_info* job, struct uint64_buff* pids);

void running_job_free(struct running_job* running);

struct running_job_buff {
    size_t capacity;
    size_t size;
    struct running_job* data;
};

struct running_job_buff running_job_buff_create();

bool running_job_buff_alloc(struct running_job_buff* buff, size_t new_capacity);

bool running_job_buff_add(struct running_job_buff* buff, struct running_job* ptr);

void running_job_buff_erase(struct running_job_buff* buff, uint32_t index);

void running_job_buff_free(struct running_job_buff* buff);