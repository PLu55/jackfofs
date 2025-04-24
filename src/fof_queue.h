#ifndef __FOF_QUEUE_H__
#define __FOF_QUEUE_H__

#include <stdint.h>
#include <fofs.h>

#include "jfofs_types.h"
#include "jfofs_private.h"
#include "debug.h"

typedef struct fof_queue_s fof_queue_t;

struct fof_queue_s
{
  uint64_t next_frame;
  uint64_t next_frame_check;
  uint64_t frame_stamp;  
  uint64_t current_slot;
  int n_slots;
  int sample_rate;  
  int buffer_size;      /* size of audio buffer, determine the size of the slots */
  DEBUG(int max_fofs);
  DEBUG(int free_cnt);
  DEBUG(int active_cnt);
  fof_t* free_fofs;              /* list of free fofs avalible to the producer */
  fof_t* first_fof;              /* list of fofs to be freed */
  fof_t* last_fof;               /* last fof in the list to be freed */
  fof_t* excess;
  fof_t** slot;
};

void fof_queue_init(fof_queue_t* q, setup_t *setup);
fof_t* fof_queue_allocate_fof(fof_queue_t* q, int *status);
void fof_queue_free_fof(fof_queue_t* q, fof_t* fof);
void fof_queue_free_fofs(fof_queue_t* q, fof_t* head, fof_t* tail);
int fof_queue_add(fof_queue_t* q, jfofs_time_t time_us, float* fof_argv);
fof_t* fof_queue_take_slot(fof_queue_t* q, int slot_idx);
#ifdef DEBUG_ENABLE
int fof_queue_check_free_list(fof_queue_t* q, int* cnt, int integrity);
#endif

#endif
