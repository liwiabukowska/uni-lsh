project('lsh', 'c')

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language : 'c')
endif

executable('lsh',
    'lsh.c',
    
    'active_job.c',
    'algorithm.c',
    'buffers.c',
    'builtin.c',
    'input.c',
    'job_info.c',
    'job_management.c',
    'lsh_exit.c',
    'parsing.c',
    'prompt.c',
    'signal_handlers.c'
)