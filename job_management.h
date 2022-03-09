#include "active_job.h"

#include <stdint.h>
#include <stdbool.h>

extern struct running_job_buff g_running_jobs;

void set_console_fg_group(struct running_job* running);

void kill_job(struct running_job* running);
void stop_job(struct running_job* running);
void interrupt_job(struct running_job* running);

void foreground_job(struct running_job* running);
void background_job(struct running_job* running);

bool wait_fg_job(struct running_job* running);

void collect_all_child_processes(void);