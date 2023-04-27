#include <stdio.h>
#include <fofs.h>
#include <math.h>

#include "unity/unity.h"

#include "process_queue.h"

void default_fof(float* a);

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_process_queue(void)
{
  double sample_rate = 48000.0;
  int n_slots = 64;
  int slot_size = 128;
  int n_free_chunks = 10;
  int chunk_size = 10;
  jfofs_status status;
  float _fof[FOF_NUMARGS];
  double time;

  fof_queue* q = fof_queue_new(sample_rate, slot_size, n_slots, n_free_chunks,
			        chunk_size, &status);
  TEST_ASSERT_NOT_NULL(q);
  TEST_ASSERT_EQUAL_INT(0, q->head);
  TEST_ASSERT_EQUAL_UINT64(0, q->current_frame);
  TEST_ASSERT_EQUAL_INT(slot_size, q->slot_size);
  TEST_ASSERT_EQUAL_INT(n_slots, q->n_slots);
  TEST_ASSERT_TRUE(fabs(sample_rate - q->sample_rate) < 1e-9);
  TEST_ASSERT_NULL(q->excess);
    
  default_fof(_fof);
  time = 0.0;
  _fof[FOF_ARG_freq] = 1.0f;
  fof_queue_add(q, time, _fof, &status);
  
  TEST_ASSERT_NOT_NULL(q->slot[0]);
  TEST_ASSERT_NULL(q->slot[0]->next);
  TEST_ASSERT_EQUAL_INT(1, q->slot[0]->size);
  TEST_ASSERT_EQUAL_INT(chunk_size, q->slot[0]->max_size);
  TEST_ASSERT_NOT_NULL(q->slot[0]->fof);
  TEST_ASSERT_TRUE(fabs(q->slot[0]->fof[0].time - time) < 1e-9);
  TEST_ASSERT_TRUE(fabsf(q->slot[0]->fof[0].argv[FOF_ARG_freq] - 1.0f) < 1e-9f);

  time = slot_size / sample_rate * 2.5;
  _fof[FOF_ARG_freq] = 2.0f;
  fof_queue_add(q, time, _fof, &status);
  TEST_ASSERT_NOT_NULL(q->slot[2]);
  
  //q->slot[0]->next
  
  //  ~171 ms period
  //  ~2.7 ms per slot
  //  slot_dt = slot_size / sample_rate;
  //  q_dt = slot_dt * slot_size;
  // fof_queue_add(q, time, data);
     
}

void default_fof(float* a)
{
  a[FOF_ARG_ampl] =     1.0f;  
  a[FOF_ARG_freq] =   100.0f;
  a[FOF_ARG_gliss] =    0.0f;
  a[FOF_ARG_phi] =      0.0f;
  a[FOF_ARG_beta] =     0.3f;
  a[FOF_ARG_alpha] =    2.5f;
  a[FOF_ARG_amin] =   0.001f;
  a[FOF_ARG_cutoff] = 0.002f;
  a[FOF_ARG_pan1] =     0.0f;
  a[FOF_ARG_pan2] =     0.0f;
  a[FOF_ARG_pan3] =     0.0f;
}
