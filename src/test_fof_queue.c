#include <unistd.h>
#include <stdio.h>
#include <fofs.h>
#include <math.h>

#include "unity/unity.h"

#include "jfofs_private.h"
#include "fof_queue.h"
#include "shmem.h"
#include "test_util.h"

void test_fof_queue_add(void)
{
  int status;
  setup_t setup;
  shmem_t* shmem;
  fof_queue_t* q;
  fof_t* fof;
  fof_t fof_in;
  uint64_t t;
    
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 32;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;

  shmem = shmem_create(&setup, &status);
  TEST_ASSERT_NOT_NULL(shmem);
  
  q = &(shmem->q);
  fof_queue_init(q, &setup);
  
  fof_default(&fof_in);
  status = fof_queue_add(q, 0UL, fof_in.argv);
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);
  TEST_ASSERT_NULL(q->slot[0]->next);
  TEST_ASSERT_EQUAL_UINT64(0UL, q->slot[0]->time_us);
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(fof_in.argv, q->slot[0]->argv, FOF_NUMARGS);

  fof = q->slot[0];
  status = fof_queue_add(q, 0UL, fof_in.argv);
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);
  TEST_ASSERT_EQUAL_PTR(fof, q->slot[0]->next);
  TEST_ASSERT_EQUAL_UINT64(0UL, q->slot[0]->time_us);
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(fof_in.argv, q->slot[0]->argv, FOF_NUMARGS);

  q->next_frame += setup.buffer_size;
  status = fof_queue_add(q, 0UL, fof_in.argv);
  TEST_ASSERT_EQUAL_INT(JFOFS_FOF_LATE_ERROR, status);

  t = jfofs_nframes_to_time(2 * setup.buffer_size + 75UL, setup.sample_rate);
  status = fof_queue_add(q, t, fof_in.argv);
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);
  TEST_ASSERT_NOT_NULL(q->slot[2]);
  TEST_ASSERT_EQUAL_UINT64(t, q->slot[2]->time_us);

  
  t = jfofs_nframes_to_time(36 * setup.buffer_size + 75UL, setup.sample_rate);
  status = fof_queue_add(q, t, fof_in.argv);
  TEST_ASSERT_EQUAL_INT(JFOFS_FOF_EXCESS_INFO, status);
  
}

void test_fof_queue_init(void)
{
  int status;
  setup_t setup;
  shmem_t* shmem;
  fof_queue_t* q;
  fof_t* fof;
  int i;
  size_t mem_size;
  size_t slots_off;
  size_t fofs_off;
  
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 32;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;

  //printf("page size: %d\n",  getpagesize());
  printf(" sizeof(fof_t): %ld\n",  sizeof(fof_t));
  printf(" sizeof(shmem_t): %ld\n",  sizeof(shmem_t));
  TEST_ASSERT_EQUAL_PTR((char*)0x7f61667f3000UL,
			shmem_aligning_ptr((char*)0x7f61667f3000UL, 64UL));
  TEST_ASSERT_EQUAL_PTR((char*)0x7f61667f3040UL,
			shmem_aligning_ptr((char*)0x7f61667f3017UL, 64UL));
  
  shmem = shmem_create(&setup, &status);
  TEST_ASSERT_NOT_NULL(shmem);

  q = &(shmem->q);

  mem_size = shmem_layout(&setup, &slots_off, &fofs_off);
  printf("mem_size: %ld slots_off: %ld fofs_off: %ld\n", mem_size, slots_off, fofs_off);
  printf("shmem: %p %p %p %p\n", shmem, (char*)shmem + slots_off,
	 (char*)shmem + fofs_off,  (char*)shmem + mem_size);

  fof_queue_init(q, &setup);

  TEST_ASSERT_EQUAL_UINT64(0UL, q->next_frame);
  TEST_ASSERT_EQUAL_UINT64(0UL, q->next_frame_check);
  TEST_ASSERT_EQUAL_INT(0, q->current_slot);
  TEST_ASSERT_EQUAL_INT(setup.n_slots, q->n_slots);
  TEST_ASSERT_EQUAL_INT(setup.sample_rate, q->sample_rate);
  TEST_ASSERT_EQUAL_INT(setup.buffer_size, q->buffer_size);
  TEST_ASSERT_NULL(q->excess);
  TEST_ASSERT_NOT_NULL(q->free_fofs);
  TEST_ASSERT_NOT_NULL(q->slot);
  TEST_ASSERT_EQUAL_PTR((char*)shmem + slots_off, q->slot);
  TEST_ASSERT_EQUAL_PTR((char*)shmem + fofs_off, q->free_fofs);
  
  for (i = 0; i < setup.n_slots; i++)
  {
    TEST_ASSERT_NULL(q->slot[i]);
  }

  fof = q->free_fofs;
  i = 0;
  while(fof)
  {
    i++;
    if (i > setup.n_max_fofs)
      break;
    TEST_ASSERT_TRUE((char*)fof + sizeof(fof_t) <= (char*)shmem + mem_size);
    fof = fof->next;
  }
  TEST_ASSERT_EQUAL_INT(setup.n_max_fofs, i);
  fof = q->free_fofs + setup.n_max_fofs - 1;
  TEST_ASSERT_NULL(fof->next);
  printf("last fof: %p\n", fof); 

#if 0
  char filename[50];
  sprintf(filename, "/proc/%d/maps", getpid());
  FILE* f = fopen(filename, "r");
  if (f == NULL)
  {
    printf("Error: failed to open file.\n");
    return;
  }
  
  char line[256];
  while (fgets(line, sizeof(line), f)) {
    printf("%s", line);
  }
  fclose(f);
#endif
  
  return;
}

#if 0

void test_fof_queue(void)
{
  int sample_rate = 48000;
  int buffer_size = 128;
  int status;
  fof_t fof;
  fof_queue_t* q;
  setup_t setup;

  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_slots = 64;
  setup.n_free_chunks = 10;
  setup.chunk_size = 10;
  
  q = fof_queue_new(sample_rate, &setup, buffer_size, &status);
  
  TEST_ASSERT_NOT_NULL(q);
  TEST_ASSERT_EQUAL_INT(0, q->head);
  TEST_ASSERT_EQUAL_UINT64(0, q->next_frame);
  TEST_ASSERT_EQUAL_INT(buffer_size, q->slot_size);
  TEST_ASSERT_EQUAL_INT(setup.n_slots, q->n_slots);
  TEST_ASSERT_TRUE(fabs(sample_rate - q->sample_rate) < 1e-9);
  TEST_ASSERT_NULL(q->excess);
    
  fof_default(&fof);
  fof.time_us = 0;
  status = fof_queue_add(q, &fof); 
 
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);
  TEST_ASSERT_NOT_NULL(q->slot[0]);
  TEST_ASSERT_NULL(q->slot[0]->next);
  TEST_ASSERT_EQUAL_INT(1, q->slot[0]->count);
  TEST_ASSERT_EQUAL_INT(setup.chunk_size, q->slot[0]->max_count);
  TEST_ASSERT_NOT_NULL(q->slot[0]->fof);
  // Should be TEST_ASSERT_EQUAL_DOUBLE but how to activate DOUBLE in Unity ?
  TEST_ASSERT_EQUAL_INT64(fof.time_us, q->slot[0]->fof[0].time_us);
  TEST_ASSERT_EQUAL_FLOAT(100.0f, q->slot[0]->fof[0].argv[FOF_ARG_freq]);
  TEST_ASSERT_TRUE(fof_equal(&fof, &q->slot[0]->fof[0]));

  fof.time_us = (buffer_size * 2500000) / sample_rate;
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
  for (int i = 2; i < setup.chunk_size + 1; i++)
  {
    fof.argv[FOF_ARG_freq] = (float) (2 + i);
    status = fof_queue_add(q, &fof);
  }
  TEST_ASSERT_TRUE(chunk != q->slot[2]);
  TEST_ASSERT_EQUAL_PTR(chunk, q->slot[2]->next);
  TEST_ASSERT_EQUAL_INT(1, q->slot[2]->count);
  TEST_ASSERT_EQUAL_INT(setup.chunk_size, q->slot[2]->next->count);

  // Fof is added to the excess slot
  fof.time_us = (buffer_size *  (setup.n_slots + 1) * 1000000ULL)
    / sample_rate;
  printf("nnn: %ld %f\n",fof.time_us, (double)fof.time_us* 1e-6); 
  fof.argv[FOF_ARG_freq] = 1000.0f;
  status = fof_queue_add(q, &fof);

  TEST_ASSERT_NOT_NULL(q->excess);
  TEST_ASSERT_NULL(q->excess->next);
  TEST_ASSERT_EQUAL_INT(1, q->excess->count);

  // Advance time and add a fof, should end up in slot 1
  q->next_frame = 48000;
  
  fof.time_us = (buffer_size + 1) * 1000000ULL / sample_rate + 1000000ULL;
  
  status = fof_queue_add(q, &fof);
  chunk = q->slot[1];
  TEST_ASSERT_NOT_NULL(chunk);
  TEST_ASSERT_EQUAL_INT(1, chunk->count);
  
  fof_queue_free(q);
  //q->slot[0]->next
  
  //  ~171 ms period
  //  ~2.7 ms per slot
  //  slot_dt = slot_size / sample_rate;
  //  q_dt = slot_dt * slot_size;
  // fof_queue_add(q, time, data);
     
}

#endif
