#include <string.h>
#include "jfofs_private.h"
#include "statistics.h" 
#include "config.h"
#include "shmem.h"

#ifdef STATISTICS_ENABLE

void statistics_init()
{
  shmem_t* shmem = shmem_ptr();

  memset((char*)&(shmem->statistics), 0, sizeof(statistics_t));
  shmem->statistics.n_slots = shmem->setup.n_slots;
}

void incr_slot_cnt(int slot)
{
  (shmem_ptr()->statistics.slot_cnt[slot])++;
}

void incr_late_cnt()
{
  shmem_ptr()->statistics.late_cnt++;
}

void incr_excess_cnt()
{
  shmem_ptr()->statistics.excess_cnt++;
}

#endif
