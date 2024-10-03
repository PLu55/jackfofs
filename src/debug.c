#include <stdio.h>
#include <fofs.h>

#include "config.h"
//#include "fofs_private.h"
#include "fof_queue.h"
#include "debug.h"

#ifdef DEBUG_ENABLE

debug_info_t debug_info;

void printf_setup(setup_t* setup)
{
  printf("   mode: %d\n", setup->mode);
  printf("   n_preallocate_fofs: %d\n", setup->n_preallocate_fofs);
  printf("   n_max_fofs: %d\n", setup->n_max_fofs);
  printf("   n_slots: %d\n", setup->n_slots);
  printf("   sample_rate: %d\n", setup->sample_rate);
  printf("   max_buffer_size: %d\n", setup->max_buffer_size);
  printf("   fofs_trace_level: %d\n", setup->fofs_trace_level);
}
void define_fof_limits(void* min, void* max)
{
  debug_info.fof_min = min;
  debug_info.fof_max = max;
  debug_info.ctrl_entry_cnt = 0;
}

void add_ctrl_entry_cnt(int i)
{
  debug_info.ctrl_entry_cnt += i;
  if (debug_info.ctrl_entry_cnt > 1)
    printf("ctrl_entry_cnt: %d", i);
}

void check_fof_addr(void* fof, int line, const char* file, const char* func)
{
  if ( fof < debug_info.fof_min || fof > debug_info.fof_max)
  {
    printf("Address of fof is bad: %p at: %d in: %s (%s)\n", fof, line, file, func);
  }
}

void check_fof_addrz(void* fof, int line, const char* file, const char* func)
{
  if ( fof != 0 && (fof < debug_info.fof_min || fof > debug_info.fof_max))
  {
    printf("Address of fof is bad: %p at: %d in: %s (%s)\n", fof, line, file, func);
  }
}

int check_free_list(fof_queue_t* q, int* cnt, int integrity)
{
  fof_t* fof;
  int fof_visited[q->max_fofs];
  void* fof_min = debug_info.fof_min;
  void* fof_max = debug_info.fof_max;
  int error_cnt;

  error_cnt = 0;
  
  for (int i = 0; i < q->max_fofs; i++)
    fof_visited[i] = 0;
  
  fof = q->free_fofs;
  while (fof)
  {
    if ( (void*)fof < fof_min || (void*)fof > fof_max)
    {
      fprintf(stderr, "check_free_list: illegal fof ptr: %p\n", fof);
      error_cnt++;
    }
    if (integrity)
    {
      ptrdiff_t o;
      o =   fof - (fof_t*) debug_info.fof_min;
      if ( o > q->max_fofs || o < 0)
      {
	fprintf(stderr, "check_free_list: illegal fof index: %ld\n", o);
	error_cnt++;
      }
    
      if (fof_visited[o] != 0)
      {
	fprintf(stderr, "check_free_list: double addess of fof!\n");
	error_cnt++;
      }
      fof_visited[o]++;
    }
    fof = fof->next;
    (*cnt)++;
    if (*cnt > q->max_fofs)
      return -1;
  }
  return error_cnt;
}

#endif
