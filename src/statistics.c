#include <string.h>
#include <stdio.h>

#include "jfofs_private.h"
#include "statistics.h" 
#include "config.h"
#include "shmem.h"

#ifdef STATISTICS_ENABLE

void statistics_init()
{
  shmem_t* shmem = shmem_ptr();
  fof_t* fof;

  memset((char*)&(shmem->statistics), 0, sizeof(statistics_t));
  shmem->statistics.n_slots = shmem->setup.n_slots;
  
  fof = shmem->q.free_fofs;
  while (fof)
  {
    fof->time_us = 0;
    fof = fof->next;
  }
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

int max_used_free_list()
{
  shmem_t* shmem = shmem_ptr();
  fof_t* fof;
  int n = 0;
  
  fof = shmem->q.free_fofs;
  while (fof)
  {
    if (fof->time_us == 0)
      break;
    n++;
    fof = fof->next;
  }
  return n;
}

void dump_statistics()
{
  shmem_t* shmem = shmem_ptr();
  statistics_t* stats = &(shmem->statistics);
  int sum = 0;

  for (int i = 0; i < stats->n_slots; i++)
    sum += stats->slot_cnt[i];
  printf("Dumping statistics:\n");
  printf("   total: %d\n", sum +  stats->late_cnt + stats->excess_cnt);
  printf("   excluding late and excess: %d\n", sum);
  printf("   late: %d\n", stats->late_cnt );
  printf("   excess: %d\n", stats->excess_cnt );
  printf("   max used fofs in queue: %d\n", max_used_free_list() );
  for (int i = 0; i < stats->n_slots; i++)
    if (stats->slot_cnt[i])
      printf("   slot[%d]: %d\n", i + 1, stats->slot_cnt[i]);
}

#endif
