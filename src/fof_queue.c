#include <stdio.h> // kan tas bort
#include <stdbool.h>
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
 * (slot) contains a list of event data items, in this perticular
 * case fof data. The number of slots must be a power of 2.
 *
 * A queue struct contains the queue array and data about the
 * queue. slot_size is the duration of a slot expressed in number of
 * samples.
 * 
 */

void fof_queue_init(fof_queue_t* q, setup_t *setup)
{
  fof_t* fof;

  q->next_frame = 0;
  q->next_frame_check = 0;
  q->current_slot = 0;
  q->n_slots = setup->n_slots;

  q->sample_rate = setup->sample_rate;
  q->buffer_size = setup->buffer_size;
  q->last_fof = NULL;
  q->excess = NULL;
  
  /* this are set by shmem: 
   *   q->slot, the slots
   *   q->free_fofs,  point to the begining of memory area for fof 
   * See shmem.h for memory layout.
   * The free fof list is not linked yet it has to be link it here.
   */
  
  for (int i = 0; i < q->n_slots; i++)
  {
    q->slot[i] = NULL;
  }
  fof = q->free_fofs;
  for (int i = 0; i < setup->n_max_fofs - 1; i++)
  {
    fof_t* next = fof + 1;
    fof->next = next;
    fof = next;
  }

  fof->next = NULL;
}

fof_t* fof_queue_allocate_fof(fof_queue_t* q, int *status)
{
  fof_t* fof;

  for(;;)
  {
    fof = __atomic_load_n(&(q->free_fofs),  __ATOMIC_ACQUIRE);

    if (fof == NULL)
    {
      *status = JFOFS_MEMORY_ERROR;
      return NULL;
    }
 
    if (__atomic_compare_exchange_n(&(q->free_fofs), &fof, fof->next, false,
				    __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      break;
  }
  fof->next = NULL;
  *status = JFOFS_SUCCESS;
  return fof;
}

void fof_queue_free_fof(fof_queue_t* q, fof_t* fof)
{
  for(;;)
  {
    fof->next = __atomic_load_n(&(q->free_fofs),  __ATOMIC_ACQUIRE);

    if (__atomic_compare_exchange_n(&(q->free_fofs), &(fof->next), fof, false,
				    __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      break;
  }
}

/* fof_queue_free_fofs is called by a thread with high priority
 * so there will be no data race.
 */
void fof_queue_free_fofs(fof_queue_t* q, fof_t* head, fof_t* tail)
{
  fof_t* fof;

  fof = __atomic_load_n(&(q->free_fofs),  __ATOMIC_ACQUIRE);
  tail->next = fof;
  __atomic_store_n(&(q->free_fofs), head, __ATOMIC_RELEASE);
}

static inline void set_fof(fof_t* fof, uint64_t time_us, float* argv)
{
  fof->time_us = time_us;
  memcpy((char*)&(fof->argv), (char*)argv, FOF_NUMARGS * sizeof(float));
}
 
int fof_queue_add(fof_queue_t* q, uint64_t time_us, float* fof_argv)
{
  fof_t* fof;
  int status;
  uint64_t start_frame;
  uint64_t next_frame;
  uint64_t next_frame_check;
  int slot_idx;

  /* TODO: release fof on faliure */

  start_frame = jfofs_time_to_nframes(time_us, q->sample_rate);
  
  /* TODO: what happens if q->next_frame is changing during this call.
   * How to syncronize  q->next_frame ? 
   * Check next_frame just before inserting the fof in a slot.
   */
  
  next_frame =__atomic_load_n(&(q->next_frame), __ATOMIC_ACQUIRE);
  
  slot_idx = (start_frame - next_frame) / q->buffer_size;
#ifdef TRACE
  printf("slot_idx: %d\n", slot_idx);
#endif
  if (slot_idx < 0)
    return JFOFS_FOF_LATE_WARNING;

  fof = fof_queue_allocate_fof(q, &status);

  if (fof == NULL)
    return JFOFS_FOF_LIMIT_ERROR;

  set_fof(fof, time_us, fof_argv);

  /* TODO: implement excess handling. */
  if (slot_idx >= q->n_slots)
  {
#ifdef TRACE
    printf("fof inserted in excess slot\n");
#endif
    fof->next = q->excess;
    q->excess = fof;
    return JFOFS_FOF_EXCESS_INFO;
  }
  else
  {
    /* Cache q->current_slot */
    if (next_frame != q->next_frame_check)
    {
#ifdef TRACE
      printf("next_frame: %ld\n", next_frame);
#endif
      q->current_slot = (next_frame / q->buffer_size) & (q->n_slots - 1);
      q->next_frame_check = next_frame;
    }
    
    slot_idx =  (slot_idx + q->current_slot) & (q->n_slots - 1);
#ifdef TRACE
    printf("q->current_slot: %ld\n", q->current_slot);
    printf("fof inserted in slot: %d\n", slot_idx);
#endif
    /* can relax if slot_idx is greater then current_slot + 1 .
     * Better to compare start_frame > next_frame +  q->buffer_size
     */
    
    if (start_frame > next_frame + q->buffer_size)
    {
#ifdef TRACE
      printf("fof inserted in fast lane\n");
#endif
      fof->next = q->slot[slot_idx];
      q->slot[slot_idx] = fof;
    }
    else
    {
      /* If fail the fof is to late so there is no need to loop,
       * but return with JFOFS_FOF_EXCESS_INFO status.
       */
      fof_t** slot_p = &(q->slot[slot_idx]);
      fof_t* slot = __atomic_load_n(slot_p, __ATOMIC_ACQUIRE);
      fof->next = slot;
      next_frame_check = __atomic_load_n(&(q->next_frame), __ATOMIC_ACQUIRE);

      if ( next_frame_check != next_frame)
	return JFOFS_FOF_LATE_WARNING;
   
      if (!__atomic_compare_exchange_n(slot_p, &slot, fof, false,
				       __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      {
	fof_queue_free_fof(q, fof);
	return JFOFS_FOF_LATE_WARNING;
      }
    }
  }
  return JFOFS_SUCCESS;
}

fof_t* fof_queue_take_slot(fof_queue_t* q, int slot_idx)
{
  return __atomic_exchange_n(&(q->slot[slot_idx]), NULL, __ATOMIC_RELEASE);
}

