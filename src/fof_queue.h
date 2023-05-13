#ifndef __FOF_QUEUE_H__
#define __FOF_QUEUE_H__

#include <stdint.h>
#include <fofs.h>

#include "jfofs_types.h"
#include "jfofs_private.h"

typedef struct fof_queue_s fof_queue_t;

struct fof_queue_s
{
  uint64_t next_frame;
  uint64_t next_frame_check;
  uint64_t current_slot;
  int n_slots;
  int sample_rate;  
  int buffer_size;
  fof_t* free_fofs;              /* list of free fofs avalible to the producer */
  fof_t* excess;
  fof_t** slot;
};

void fof_queue_init(fof_queue_t* q, setup_t *setup);
int fof_queue_add(fof_queue_t* q, jfofs_time_t time_us, float* fof_argv);

#endif
