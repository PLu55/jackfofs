#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "config.h"

#ifdef DEBUG_ENABLE

struct debug_info_s
{
  void* fof_min;
  void* fof_max;
  int ctrl_entry_cnt;
};

typedef struct debug_info_s debug_info_t;

extern debug_info_t debug_info;

void check_fof_addr(void* fof, int line, const char* file, const char* func);
void check_fof_addrz(void* fof, int line, const char* file, const char* func);
int check_free_list(fof_queue_t* q, int* cnt, int integrity);
void define_fof_limits(void* min, void* max);
void add_ctrl_entry_cnt(int i);

#define DEBUG(what) what
#define CHECK_FOF_ADDR(fof) check_fof_addr(fof, __LINE__, __FILE__, __func__)
#define CHECK_FOF_ADDR_OR_ZERO(fof) check_fof_addrz(fof, __LINE__, __FILE__, __func__)
#define DEFINE_FOF_LIMITS(min, max) define_fof_limits(min, max)
#define ADD_CTRL_ENTRY_CNT(i) add_ctrl_entry_cnt(i)
		   
#else

#define DEBUG(what)
#define CHECK_FOF_ADDR(fof)
#define CHECK_FOF_ADDR_OR_ZERO(fof)
#define DEFINE_FOF_LIMITS(min, max)
#define ADD_CTRL_ENTRY_CNT(i)

#endif

#endif
