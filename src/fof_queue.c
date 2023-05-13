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
  q->excess = NULL;
    printf("A\n");
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
    printf("B\n");
  fof = q->free_fofs;
  for (int i = 0; i < setup->n_max_fofs - 1; i++)
  {
    fof_t* next = fof + 1;
    fof->next = next;
    fof = next;
  }
    printf("C\n");
  fof->next = NULL;
}

fof_t* get_free_fof(fof_queue_t* q, int *status)
{
  fof_t* fof;

  fof = q->free_fofs;
  if (fof == NULL)
  {
    *status = JFOFS_MEMORY_ERROR;
    return NULL;
  }
  q->free_fofs = fof->next;
  fof->next = NULL;

  return fof;
}

void fof_queue_add_free_fofs(fof_queue_t* q, fof_t* head, fof_t* tail)
{
  tail->next = q->free_fofs;
  q->free_fofs = head;
}

int fof_queue_add(fof_queue_t* q, uint64_t time_us, float* fof_argv)
{
  fof_t* fof;
  int status;
  uint64_t frame;
  int slot_idx;
  
  fof = get_free_fof(q, &status);

  if (fof == NULL)
  {
    return JFOFS_FOF_LIMIT_ERROR;
  }
  frame = jfofs_time_to_nframes(time_us, q->sample_rate);
  slot_idx = ((frame - q->next_frame) / q->buffer_size);
  printf("slot_idx: %d\n", slot_idx);
  if (slot_idx < 0)
    return JFOFS_FOF_LATE_ERROR;

  fof->time_us = time_us;
  memcpy((char*)&(fof->argv), (char*)fof_argv, FOF_NUMARGS * sizeof(float));

  if (slot_idx >= q->n_slots)
  {
    printf("fof inserted in excess slot\n");
    fof->next = q->excess;
    q->excess = fof;
    return JFOFS_FOF_EXCESS_INFO;
  }
  else
  {
    if (q->next_frame != q->next_frame_check)
    {
      printf("q->next_frame: %ld\n", q->next_frame);
      q->current_slot = (q->next_frame / q->buffer_size) & (q->n_slots - 1);
      q->next_frame_check = q->next_frame;
    }
    slot_idx =  (slot_idx + q->current_slot) & (q->n_slots - 1);
    printf("q->current_slot: %ld\n", q->current_slot);
    printf("fof inserted in slot: %d\n", slot_idx);

    /* can relax if slot_idx is greater then current_slot + 1 .
     * Better to compare frame > q->next_frame +  q->buffer_size
     */
    
    if (frame > q->next_frame +  q->buffer_size)
    {
       printf("fof inserted in fast lane\n");
       fof->next = q->slot[slot_idx];
       q->slot[slot_idx] = fof;
    }
    else
    {
      /* TODO: if fail the fof is to late so there is no need to loop,
       * but return with JFOFS_FOF_EXCESS_INFO status.
       */
      fof_t** slot_p = &(q->slot[slot_idx]);
      while(true)
      {
	  fof->next = *slot_p;
	  if (__atomic_compare_exchange_n(slot_p, slot_p, fof, false,
					  __ATOMIC_RELEASE, __ATOMIC_RELAXED))
	    break;
      }
    }
  }
  return JFOFS_SUCCESS;
}


