#ifndef __jfofs_process_queue__
#define __jfofs_process_queue__

#include <stdint.h>
#include <fofs.h>

#include "jfofs_types.h"
#include "jfofs_private.h"

struct chunk_s
{
  chunk* next;
  int size;
  int max_size;
  fof* fof;
  void* pad[5];
};

typedef struct fof_queue_s fof_queue;

struct fof_queue_s
{
  int head;               /* point to the slot to be processed next */
  uint64_t next_frame;    /* time of the current slot in samples */  
  int n_slots;            /* number of slots in the queue */
  int slot_size;          /* duration of a slot in number of samples */
  int chunk_size;         /* number of items each chunk can hold */
  double sample_rate;     /* sample rate */
  int n_free_chunks;      /* number of free chunks to allocate */
  chunk* free_chunks;     /* linked list of free chunks */
  chunk* excess;          /* events that are outside of the range of the slots */
  chunk** slot;           /* array holding the slots */
};

fof_queue* fof_queue_new(int sample_rate, int n_slots, int slot_size,
			 int n_free_chunks, int chunk_size, int* status);
chunk* chunk_new(fof_queue* q, int* status);
void chunk_free(chunk* ch);
chunk* fof_queue_new_chunk(fof_queue* q, chunk** chunk_p, int* status);
void fof_queue_chunk_free(fof_queue* q, chunk* ch);
int fof_queue_add(fof_queue* q, fof* fof_in);
chunk* fof_queue_remove_chunk_at_head(fof_queue* q, int cidx);

#endif
