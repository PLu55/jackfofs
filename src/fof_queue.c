#include <stdio.h> // kan tas bort

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "jfofs_types.h"
#include "fof_queue.h"

/* fof_queue is an implementation of a circular queue to handle
 * scheduling problems in audio processing.
 * 
 * The main data structure of the queue is an array where each cell
 * (slot) contains a list of chunks holding the event data items. The
 * number of slots must be a power of 2.
 *
 * A chunk is an array holding the actual events. When a chunk is full
 * a new chunk is allocated and linked in to the list of chunks in a
 * slot.
 * 
 * A queue struct contains the queue array and data about the
 * queue. slot_size is the duration of a slot expressed in number of
 * samples.
 * 
 */

// max_ahead ???

// current_buffer_size see jack_set_buffer_size_callback()

// 
// float fof_vector[FOF_NUMARGS];

void allocate_chunks(fof_queue* q, int n_chunks, int* status);
  
/* fof_queue_new():
 * Allocate a new queue. 
 * size must be power of 2
 * current slot is the currently processed slot, no write access is allowed in
 * this slot.  
 */

fof_queue* fof_queue_new(int sample_rate, int n_slots, int slot_size,
			 int n_free_chunks, int chunk_size, int* status)
{
  fof_queue* q;
  
  *status = posix_memalign((void**) &q, CACHE_LINE_SIZE,
			 sizeof(fof_queue) + sizeof(chunk*) * n_slots);

  if (q == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  
  q->head = 0;
  q->next_frame = 0;
  q->n_slots = n_slots;
  q->slot_size = slot_size;
  q->chunk_size = chunk_size;
  q->sample_rate = sample_rate;
  q->n_free_chunks = n_free_chunks;
  q->free_chunks = NULL;
  q->excess = NULL;
  q->slot = (chunk**)(&q->slot + 1);
  
  allocate_chunks(q, n_free_chunks, status);
  if (*status != JFOFS_SUCCESS)
    return NULL;
  
  *status = JFOFS_SUCCESS;
  return q;
}

void fof_queue_free(fof_queue* q)
{
  while(q->free_chunks != NULL)
  {
    chunk* ch = q->free_chunks;
    q->free_chunks = ch->next;  
    chunk_free(ch);
  }
  // TODO: free all chunks!!!
  free(q);
}

chunk* chunk_allocate(fof_queue* q, int* status)
{
  chunk* ch;

  *status = posix_memalign((void**) &ch, CACHE_LINE_SIZE,
			  sizeof(chunk) + sizeof(fof) * q->chunk_size);

  if (ch == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  ch->next = NULL;
  ch->count = 0;
  ch->max_count = q->chunk_size;
  ch->fof = (fof*)(ch + 1);

  *status = JFOFS_SUCCESS;
  return ch;
}

void chunk_free(chunk* ch)
{
  free(ch);
}

chunk* get_free_chunk(fof_queue* q)
{
  chunk* _chunk;

  _chunk = q->free_chunks;
  __atomic_thread_fence(__ATOMIC_ACQ_REL);
  q->free_chunks->next = _chunk->next;
  _chunk->next = NULL;
  _chunk->count = NULL;
  return _chunk;
}

void allocate_chunks(fof_queue* q, int n_chunks, int* status)
{
  chunk* _chunk;
  chunk* chunk_head = NULL;
  chunk* chunk_tail = NULL;
  
  for (int i = 0; i < n_chunks; i++)
  {
    _chunk = chunk_allocate(q, status);
    if (_chunk == NULL)
      return;
    if ( chunk_tail == NULL)
      chunk_tail = _chunk;
    _chunk->next = chunk_head;
    chunk_head = _chunk;
  }
  chunk_tail->next = q->free_chunks;
  __atomic_thread_fence(__ATOMIC_ACQ_REL); 
  q->free_chunks = chunk_head;
  *status = JFOFS_SUCCESS;
}

chunk* fof_queue_new_chunk(fof_queue* q, chunk** chunk_p, int* status)
{
  chunk* _chunk;

  if (q->free_chunks == NULL)
  {
    allocate_chunks(q, q->n_free_chunks, status);
    if (*status != JFOFS_SUCCESS)
      return NULL;
    q->n_free_chunks *= 2;
  }
  
  _chunk = get_free_chunk(q);
  _chunk->count = 0;
  _chunk->next = *chunk_p;
  *chunk_p = _chunk;
 
  *status = JFOFS_SUCCESS;
  return _chunk;
}

void fof_queue_chunk_free(fof_queue* q, chunk* ch)
{
  ch->next = q->free_chunks;
  q->free_chunks = ch;
}

chunk* chunk_add_fof(fof_queue* q, chunk** chunk, fof* fof_in, int* status)
{
  if (*chunk == 0 || (*chunk)->count == (*chunk)->max_count)
  {
      *chunk = fof_queue_new_chunk(q, chunk, status);
      if (*chunk == NULL)
	return NULL;
  }
  fof* fof_p = &(*chunk)->fof[(*chunk)->count++];
  memcpy(fof_p, fof_in, sizeof(fof));
  return *chunk;
}

int fof_queue_add(fof_queue* q, fof* fof_in)
{
  int status;
  int slot;
  uint64_t n;
  //printf("fof_queue_add\n");

  n = __atomic_load_n(&q->next_frame, __ATOMIC_ACQUIRE);
  
  slot = ((int) rint(fof_in->time * q->sample_rate) - n) / q->slot_size;
  //printf("   slot: %d\n", slot);
  if (slot < q->n_slots)
  {
    slot = (q->head + slot) & (q->n_slots - 1);

    if (chunk_add_fof(q, &(q->slot[slot]), fof_in, &status) == NULL)
      return status;
  }
  else
  {
    if (chunk_add_fof(q, &(q->excess), fof_in, &status) == NULL)
      return status;
  }
  return JFOFS_SUCCESS;
}

chunk* fof_queue_remove_chunk_at_head(fof_queue* q, int cidx)
{
  chunk* chunk = q->slot[cidx];
  
  q->slot[cidx] = chunk->next;
  chunk->next = NULL;
  return chunk;
}

// TODO: implement excess handling
