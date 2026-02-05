#include <stdio.h> // kan tas bort
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "jfofs_types.h"
#include "fof_queue.h"
#include "config.h"
#include "debug.h"
#include "statistics.h"

/* fof_queue is an implementation of a circular queue to handle
 * scheduling problems in audio processing.
 *
 * The main data structure of the queue is an array where each cell
 * (slot) contains a list of event data items, in this perticular
 * case fof data. The number of slots must be a power of 2.
 *
 * A queue struct contains the queue array and data about the
 * queue. buffer_size is the duration of a slot expressed in number of
 * samples.
 *
 */

void fof_queue_init(fof_queue_t *q, setup_t *setup)
{
  fof_t *fof;

  q->next_frame = 0;
  q->next_frame_check = 0;
  q->current_slot = 0;
  q->n_slots = setup->n_slots;

#ifdef DEBUG_ENABLE
  q->max_fofs = setup->n_max_fofs;
  q->free_cnt = setup->n_max_fofs;
#endif

  q->sample_rate = setup->sample_rate;
  q->buffer_size = setup->max_buffer_size;
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
    fof_t *next = fof + 1;
    fof->next = next;
    fof = next;
  }

  fof->next = NULL;
}

fof_t *fof_queue_allocate_fof(fof_queue_t *q, int *status)
{
  fof_t *fof;

  for (;;)
  {
    fof = __atomic_load_n(&(q->free_fofs), __ATOMIC_ACQUIRE);

    if (fof == NULL)
    {
      *status = JFOFS_MEMORY_ERROR;
      return NULL;
    }

    CHECK_FOF_ADDR(fof);

    if (__atomic_compare_exchange_n(&(q->free_fofs), &fof, fof->next, false,
                                    __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      break;
  }

  DEBUG(q->free_cnt--);
  fof->next = NULL;
  *status = JFOFS_SUCCESS;
  return fof;
}

void fof_queue_free_fof(fof_queue_t *q, fof_t *fof)
{
  CHECK_FOF_ADDR(fof);
  DEBUG(q->free_cnt++);
  for (;;)
  {
    fof->next = __atomic_load_n(&(q->free_fofs), __ATOMIC_ACQUIRE);

    if (__atomic_compare_exchange_n(&(q->free_fofs), &(fof->next), fof, false,
                                    __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      break;
  }
}

void fof_queue_free_fofs(fof_queue_t *q, fof_t *head, fof_t *tail)
{
  fof_t *fof;

#ifdef DEBUG_ENABLE
  fof = head;
  while (fof)
  {
    q->free_cnt++;
    if (fof == tail)
      break;
    fof = fof->next;
  }
  if (fof != tail)
    fprintf(stderr, "Debug: fof_queue_free_fofs, inconsistant call!\n");
#endif

  for (;;)
  {
    fof = __atomic_load_n(&(q->free_fofs), __ATOMIC_ACQUIRE);
    tail->next = fof;
    /* __atomic_store_n(&(q->free_fofs), head, __ATOMIC_RELEASE); */
    if (__atomic_compare_exchange_n(&(q->free_fofs), &(tail->next), head, false,
                                    __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      break;
  }
  CHECK_FOF_ADDR_OR_ZERO(fof);
  CHECK_FOF_ADDR(head);
  CHECK_FOF_ADDR(tail);
}

static inline void set_fof(fof_t *fof, uint64_t time_us, const float *argv)
{
  fof->time_us = time_us;
  memcpy((char *)&(fof->argv), (char *)argv, FOF_NUMARGS * sizeof(float));
}

int fof_queue_add(fof_queue_t *q, uint64_t time_us, const float *argv)
{
  fof_t *fof;
  int status;
  uint64_t start_frame;
  uint64_t next_frame;
  uint64_t next_frame_check;
  int slot_idx;
  int slot;

  fof = fof_queue_allocate_fof(q, &status);

  if (fof == NULL)
    return JFOFS_FOF_LIMIT_ERROR;

  start_frame = jfofs_time_to_nframes(time_us, q->sample_rate);

  set_fof(fof, time_us, argv);

  next_frame = __atomic_load_n(&(q->next_frame), __ATOMIC_ACQUIRE);

recalculate:
  slot_idx = (start_frame - next_frame) / q->buffer_size;

#ifdef TRACE
  printf("slot_idx: %d\n", slot_idx);
#endif

  if (slot_idx < 0)
  {
    INCR_LATE_CNT();
    fof_queue_free_fof(q, fof);
    return JFOFS_FOF_LATE_WARNING;
  }

  /* TODO: implement excess handling. */
  /* Case excess */
  if (slot_idx >= q->n_slots)
  {
#ifdef TRACE
    printf("fof inserted in excess slot\n");
#endif
    INCR_EXCESS_CNT();
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

    slot = (slot_idx + q->current_slot) & (q->n_slots - 1);
#ifdef TRACE
    printf("q->current_slot: %ld\n", q->current_slot);
    printf("fof inserted in slot: %d\n", slot);
#endif

    /* Case: add fof to a slot that is not the current slot
     *   The assumption that we can relax if slot_idx is greater then current_slot + 1
     *   is not valid, the access must be protected!
     *
     *   A better algorithm is needed, now a fof can end up in the wrong slot.
     *   It's not a catastrophy as the case is handled by the fofs library.
     */

    if (start_frame > next_frame + q->buffer_size)
    {
      fof_t **slot_p = &(q->slot[slot]);

#ifdef TRACE
      printf("fof inserted in fast lane\n");
#endif

      for (;;)
      {
        fof->next = __atomic_load_n(slot_p, __ATOMIC_ACQUIRE);

        next_frame_check = __atomic_load_n(&(q->next_frame), __ATOMIC_ACQUIRE);
        if (next_frame_check != next_frame)
        {
          next_frame = next_frame_check;
          goto recalculate;
        }

        /* Here is the glitch where a fof can end up in the wrong slot.
         * It happens when q->next_frame is changed between the access
         * above and the setting below.
         */

        if (__atomic_compare_exchange_n(slot_p, &(fof->next), fof, false,
                                        __ATOMIC_RELEASE, __ATOMIC_RELAXED))
        {
          break;
        }
      }
      INCR_SLOT_CNT(slot_idx);
    }
    else
    {
      /* Case: add fof to the current slot (next to be consumed)
       *   If fail the fof is to late so there is no need to loop,
       *   but return with JFOFS_FOF_EXCESS_INFO status.
       */
      fof_t **slot_p = &(q->slot[slot]);
      fof_t *slot = __atomic_load_n(slot_p, __ATOMIC_ACQUIRE);

      fof->next = slot;
      next_frame_check = __atomic_load_n(&(q->next_frame), __ATOMIC_ACQUIRE);

      if (next_frame_check != next_frame)
      {
        fof_queue_free_fof(q, fof);
        INCR_LATE_CNT();
        return JFOFS_FOF_LATE_WARNING;
      }

      if (!__atomic_compare_exchange_n(slot_p, &slot, fof, false,
                                       __ATOMIC_RELEASE, __ATOMIC_RELAXED))
      {
        fof_queue_free_fof(q, fof);
        INCR_LATE_CNT();
        return JFOFS_FOF_LATE_WARNING;
      }
      INCR_SLOT_CNT(slot_idx);
    }
  }
  return JFOFS_SUCCESS;
}

fof_t *fof_queue_take_slot(fof_queue_t *q, int slot)
{
  return __atomic_exchange_n(&(q->slot[slot]), NULL, __ATOMIC_RELEASE);
}
