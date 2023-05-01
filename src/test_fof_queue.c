#include <stdio.h>
#include <fofs.h>
#include <math.h>

#include "unity/unity.h"

#include "jfofs_private.h"
#include "fof_queue.h"
#include "test_util.h"

int fof_equal(fof* fof1, fof* fof2);

void test_fof_queue_chunk_handling(void)
{
  int sample_rate = 48000;
  int n_slots = 64;
  int slot_size = 128;
  int n_free_chunks = 10;
  int chunk_size = 16;
  int status;
  fof fof;
  fof_queue* q;
  chunk* chk;
  
  TEST_ASSERT_EQUAL_UINT64(64, sizeof(chunk));
  TEST_ASSERT_EQUAL_UINT64(64, sizeof(fof));
    
  q = fof_queue_new(sample_rate, n_slots, slot_size, n_free_chunks, chunk_size,
		    &status);
   TEST_ASSERT_NOT_NULL(q);

   chk = q->free_chunks;
   for (int i = 0; i < n_free_chunks; i++)
   {
     TEST_ASSERT_NOT_NULL(chk);
     chk = chk->next;
   }
   TEST_ASSERT_NULL(chk);

   chk = get_free_chunk(q);
   TEST_ASSERT_NOT_EQUAL(q->free_chunks, chk);
   TEST_ASSERT_NULL(chk->next);
   
   add_chunks_to_free_list(q, chk, chk);
   TEST_ASSERT_EQUAL(q->free_chunks, chk);
   TEST_ASSERT_NOT_NULL(chk->next);
}

void test_fof_queue(void)
{
  int sample_rate = 48000;
  int n_slots = 64;
  int slot_size = 128;
  int n_free_chunks = 10;
  int chunk_size = 10;
  int status;
  fof fof;
  fof_queue* q;

  q = fof_queue_new(sample_rate, n_slots, slot_size, n_free_chunks, chunk_size,
		    &status);
  
  TEST_ASSERT_NOT_NULL(q);
  TEST_ASSERT_EQUAL_INT(0, q->head);
  TEST_ASSERT_EQUAL_UINT64(0, q->next_frame);
  TEST_ASSERT_EQUAL_INT(slot_size, q->slot_size);
  TEST_ASSERT_EQUAL_INT(n_slots, q->n_slots);
  TEST_ASSERT_TRUE(fabs(sample_rate - q->sample_rate) < 1e-9);
  TEST_ASSERT_NULL(q->excess);
    
  fof_default(&fof);
  fof.time_us = 0;
  status = fof_queue_add(q, &fof); 
 
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);
  TEST_ASSERT_NOT_NULL(q->slot[0]);
  TEST_ASSERT_NULL(q->slot[0]->next);
  TEST_ASSERT_EQUAL_INT(1, q->slot[0]->count);
  TEST_ASSERT_EQUAL_INT(chunk_size, q->slot[0]->max_count);
  TEST_ASSERT_NOT_NULL(q->slot[0]->fof);
  // Should be TEST_ASSERT_EQUAL_DOUBLE but how to activate DOUBLE in Unity ?
  TEST_ASSERT_EQUAL_INT64(fof.time_us, q->slot[0]->fof[0].time_us);
  TEST_ASSERT_EQUAL_FLOAT(100.0f, q->slot[0]->fof[0].argv[FOF_ARG_freq]);
  TEST_ASSERT_TRUE(fof_equal(&fof, &q->slot[0]->fof[0]));

  fof.time_us = (slot_size * 2500000) / sample_rate;
  fof.argv[FOF_ARG_freq] = 2.0f;
  status = fof_queue_add(q, &fof);
  
  TEST_ASSERT_NOT_NULL(q->slot[2]);
  TEST_ASSERT_EQUAL_INT(1, q->slot[2]->count);
  TEST_ASSERT_FLOAT_WITHIN(1e-9f, 2.0f, q->slot[2]->fof[0].argv[FOF_ARG_freq]);

  fof.argv[FOF_ARG_freq] = 3.0f;
  status = fof_queue_add(q, &fof);
  TEST_ASSERT_NOT_NULL(q->slot[2]);
  TEST_ASSERT_FLOAT_WITHIN(1e-9f, 3.0f, q->slot[2]->fof[1].argv[FOF_ARG_freq]);

  // Overflow a chunk to force a new chunk to be added. 
  chunk* chunk = q->slot[2];
  for (int i = 2; i < chunk_size + 1; i++)
  {
    fof.argv[FOF_ARG_freq] = (float) (2 + i);
    status = fof_queue_add(q, &fof);
  }
  TEST_ASSERT_TRUE(chunk != q->slot[2]);
  TEST_ASSERT_EQUAL_PTR(chunk, q->slot[2]->next);
  TEST_ASSERT_EQUAL_INT(1, q->slot[2]->count);
  TEST_ASSERT_EQUAL_INT(chunk_size, q->slot[2]->next->count);

  // Fof is added to the excess slot
  fof.time_us = (slot_size *  (n_slots + 1) * 1000000ULL) / sample_rate;
  printf("nnn: %ld %f\n",fof.time_us, (double)fof.time_us* 1e-6); 
  fof.argv[FOF_ARG_freq] = 1000.0f;
  status = fof_queue_add(q, &fof);

  TEST_ASSERT_NOT_NULL(q->excess);
  TEST_ASSERT_NULL(q->excess->next);
  TEST_ASSERT_EQUAL_INT(1, q->excess->count);

  // Advance time and add a fof, should end up in slot 1
  q->next_frame = 48000;
  
  fof.time_us = (slot_size + 1) * 1000000ULL / sample_rate + 1000000ULL;
  
  status = fof_queue_add(q, &fof);
  chunk = q->slot[1];
  TEST_ASSERT_NOT_NULL(chunk);
  TEST_ASSERT_EQUAL_INT(1, chunk->count);
  
  //q->slot[0]->next
  
  //  ~171 ms period
  //  ~2.7 ms per slot
  //  slot_dt = slot_size / sample_rate;
  //  q_dt = slot_dt * slot_size;
  // fof_queue_add(q, time, data);
     
}

int fof_equal(fof* fof1, fof* fof2)
{
  int r = 1;
  for (int i = 0; i < FOF_NUMARGS; i++)
  {
    r = r && fof1->argv[i] == fof2->argv[i];
  }
  
  return
    r && fof1->time_us == fof2->time_us;
}
