#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "config.h"


#ifdef DEBUG_ENABLE

#include "jfofs_private.h"

void check_fof_addr(fof_t* fof, int line, const char* file, const char* func);
void define_fof_limits(void* min, void* max);
void add_ctrl_entry_cnt(int i);

#define CHECK_FOF_ADDR(fof) check_fof_addr(fof, __LINE__, __FILE__, __func__)
#define DEFINE_FOF_LIMITS(min, max) define_fof_limits(min, max)
#define ADD_CTRL_ENTRY_CNT(i) add_ctrl_entry_cnt(i)
		   
#else

#define CHECK_FOF_ADDR(fof)
#define DEFINE_FOF_LIMITS(min, max)
#define ADD_CTRL_ENTRY_CNT(i)

#endif

#endif
