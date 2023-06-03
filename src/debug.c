#include <stdio.h>

#include "config.h"

#ifdef DEBUG_ENABLE

struct debug_info_s
{
  void* fof_min;
  void* fof_max;
  int ctrl_entry_cnt;
};

typedef struct debug_info_s debug_info_t;

static debug_info_t debug_info;

void define_fof_limits(void* min, void* max)
{
  debug_info.fof_min = min;
  debug_info.fof_max = max;
  debug_info.ctrl_entry_cnt = 0;
}

void check_fof_addr(void* fof, int line, const char* file, const char* func)
{
  if ( fof < debug_info.fof_min || fof > debug_info.fof_max)
  {
    printf("Address of fof is bad: %p at: %d in: %s (%s)\n", fof, line, file, func);
  }
}

void add_ctrl_entry_cnt(int i)
{
  debug_info.ctrl_entry_cnt += i;
  if (debug_info.ctrl_entry_cnt > 1)
    printf("ctrl_entry_cnt: %d", i);
}

#endif
