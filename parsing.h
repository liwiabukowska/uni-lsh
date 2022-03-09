#pragma once

#include "buffers.h"
#include "job_info.h"

#include <stdint.h>

struct ptr_buff parse_job_argv(char* begin, char* end);

struct job_info_buff parse(char* begin, char* end);