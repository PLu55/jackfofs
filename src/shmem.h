#ifndef __SHM_H_
#define __SHM_H_

#include "fof_queue.h"
#include "manager.h"
#include "config.h"
#include "statistics.h"

/* memory layout:

 *************************
 shmem_t                   
 *************************
 fof_t[n_slots]            <--- q->slot
 *************************
 fof_t[n_max_fofs]         
 **************************/

typedef struct shmem_s shmem_t;
typedef struct shmem_offsets_s shmem_offsets_t;

struct shmem_offsets_s
{
  size_t slots;
  size_t fofs;
  size_t size;
};

struct shmem_s
{
  shmem_t* base;
  size_t size;
  setup_t setup;
  int has_statistics;
  STATISTICS_T(statistics);
  fof_queue_t q;
};

/* TODO: replace setup with n_slots and n_max_fofs */
shmem_t* shmem_create(setup_t* setup, int* status);
shmem_t* shmem_link(int* status);
void shmem_unlink(shmem_t* shmem);
void shmem_unmap(shmem_t* shmem);
char* shmem_aligning_ptr(char* ptr, size_t alignment_size);
size_t shmem_layout(setup_t* setup, size_t* slots_off, size_t* fofs_off);
shmem_t* shmem_ptr(void);

#endif
