#include <stdio.h> // kan tas bort

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "jfofs_types.h"
#include "process_queue.h"

/* process_queue is an implementation of a circular queue to handle
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


void allocate_chunks(fof_queue* q, jfofs_status* status);
  
/* fof_queue_new():
 * Allocate a new queue. 
 * size must be power of 2
 * current chunk is the currently processed chunk, no write access is allowed in
 * this chunk.  
 */

fof_queue* fof_queue_new(double sample_rate, int slot_size, int n_slots,
			 int n_free_chunks, int chunk_size, jfofs_status* status)
{
  fof_queue* q;
  int status;
  
  status= posix_memalign((void**) &q, CACHE_LINE_SIZE,
			 sizeof(fof_queue) + sizeof(chunk*) * size);
		 
  if (q == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  
  q->head = 0;
  q->current_frame = 0;
  q->n_slots = n_slots;
  q->slot_size = slot_size;
  q->chunk_size = chunk_size;
  q->sample_rate = sample_rate;
  q->n_free_chunks = n_free_chunks;
  q->free_chunks = NULL;
  q->excess = NULL;
  q->slot = (chunk**)(&q->slot + 1);
  
  allocate_chunks(q, status);
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

chunk* chunk_allocate(fof_queue* q, jfofs_status* status)
{
  chunk* ch;

  &status = posix_memalign((void**) &ch, CACHE_LINE_SIZE,
			  sizeof(chunk) + sizeof(fof) * q->chunk_size);

  if (ch == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  ch->next = NULL;
  ch->size = 0;
  ch->max_size = q->chunk_size;
  ch->fof = (fof*)(&ch->fof + 1);
  
  *status = JFOFS_SUCCESS;
  return ch;
}

void chunk_free(chunk* ch)
{
  free(ch);
}

void allocate_chunks(fof_queue* q, jfofs_status* status)
{
  printf("allocate_chunks ...");
  chunk* _chunk;
  
  for (int i = 0; i < q->n_free_chunks; i++)
  {
    _chunk = chunk_allocate(q, status);
    if (_chunk == NULL)
      return;
    _chunk->next = q->free_chunks;
    q->free_chunks = _chunk;
  }
  *status = JFOFS_SUCCESS;
  printf(" success!\n");
}

chunk* fof_queue_new_chunk(fof_queue* q, chunk** chunk_p, jfofs_status* status)
{
  printf("fof_queue_new_chunk\n");
  chunk* _chunk;

  if (q->free_chunks == NULL)
  {
    allocate_chunks(q, status);
    if (*status != JFOFS_SUCCESS)
      return NULL;
    q->n_free_chunks *= 2;
  }

  _chunk = q->free_chunks;
  q->free_chunks = _chunk->next;
  
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


chunk* chunk_add_fof(fof_queue* q, chunk** chunk, fof* fof_in, jfofs_status* status)
{
  if (*chunk == 0 || (*chunk)->size == (*chunk)->max_size)
  {
      *chunk = fof_queue_new_chunk(q, chunk, status);
      if (*chunk == NULL)
	return NULL;
  }
  fof* fof_p = &(*chunk)->fof[(*chunk)->size++];
  memcpy(fof_p, fof_in, sizeof(fof));
  return *chunk;
}

jfofs_status fof_queue_add(fof_queue* q, fof* fof_in)
{
  jfofs_status status;
  
  printf("fof_queue_add\n");

  fof* _fof;
      
  int slot = ((int) rint(fof_in->time * q->sample_rate) - q->current_frame) /
             q->slot_size;
  printf("   slot: %d\n", slot);
  if (slot < q->size)
  {
    slot = (q->head + slot) & (q->size - 1);
    chunk* chunk = q->slot[slot];

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

