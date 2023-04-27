#ifndef __jfofs_process_queue__
#define __jfofs_process_queue__

#include <stdint.h>
#include <fofs.h>

#include "jfofs_types.h"

typedef struct fof_s fof;

struct fof_s
{
  double time;
  float argv[FOF_NUMARGS];
};

typedef struct chunk_s chunk;

struct chunk_s
{
  chunk* next;
  int size;
  int max_size;
  fof* fof;
};

typedef struct fof_queue_s fof_queue;

struct fof_queue_s
{
  int head;               /* point to the slot to be processed next */
  uint64_t current_frame; /* time of the current slot in samples */  
  int n_slots;            /* number of slots in the queue */
  int slot_size;          /* duration of a slot in number of samples */
  int chunk_size;         /* number of items each chunk can hold */
  double sample_rate;     /* sample rate */
  int n_free_chunks;      /* number of free chunks to allocate */
  chunk* free_chunks;     /* linked list of free chunks */
  chunk* excess;          /* events that are outside of the range of the slots */
  chunk** slot;           /* array holding the slots */
};

fof_queue* fof_queue_new(double sample_rate, int slot_size, int size,
			 int n_free_chunks, int chunk_size, jfofs_status* status);
chunk* chunk_new(fof_queue* q, jfofs_status* status);
void chunk_free(chunk* ch);
chunk* fof_queue_new_chunk(fof_queue* q, chunk** chunk_p, jfofs_status* status);
void fof_queue_chunk_free(fof_queue* q, chunk* ch);
jfofs_status fof_queue_add(fof_queue* q, fof* fof_in);
chunk* fof_queue_remove_chunk_at_head(fof_queue* q, int cidx);

#endif
