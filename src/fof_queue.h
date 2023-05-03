#ifndef __FOF_QUEUE_H__
#define __FOF_QUEUE_H__

#include <stdint.h>
#include <fofs.h>

#include "jfofs_types.h"
#include "jfofs_private.h"

struct chunk_s
{
  chunk* next;
  int count;
  int max_count;
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
  uint64_t sample_rate;   /* sample rate */
  int n_free_chunks;      /* number of free chunks to allocate */
  chunk* free_chunks;     /* linked list of free chunks */
  chunk* excess;          /* events that are outside of the range of the slots */
  chunk** slot;           /* array holding the slots */
};

fof_queue* fof_queue_new(int sample_rate, setup *_setup, int buffer_size, int* status);
chunk* chunk_new(fof_queue* q, int* status);
void fof_queue_free(fof_queue* q);
chunk* fof_queue_new_chunk(fof_queue* q, chunk** chunk_p, int* status);
void fof_queue_chunk_free(fof_queue* q, chunk* ch);
int fof_queue_add(fof_queue* q, fof* fof_in);
chunk* fof_queue_remove_chunk_at_head(fof_queue* q, int cidx);

static inline chunk* get_free_chunk(fof_queue* q);
static inline void add_chunks_to_free_list(fof_queue* q, chunk* head_chunk,
					   chunk* tail_chunk);

static inline chunk* get_free_chunk(fof_queue* q)
{
  chunk* _chunk;

  _chunk = q->free_chunks;
  __atomic_thread_fence(__ATOMIC_ACQ_REL);
  q->free_chunks = _chunk->next;
  _chunk->next = NULL;
  _chunk->count = 0;
  return _chunk;
}

static inline void add_chunks_to_free_list(fof_queue* q, chunk* head_chunk,
					   chunk* tail_chunk)
{
  tail_chunk->next = q->free_chunks;
  __atomic_thread_fence(__ATOMIC_ACQ_REL); 
  q->free_chunks = head_chunk;
}

#endif
