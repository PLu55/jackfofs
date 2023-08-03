#include <unistd.h>
#include <stdio.h>
#include <unity/unity.h>
#include <jack/jack.h>
#include <fofs.h>

#include "jfofs.h"
#include "test_util.h"
#include "dsp_client.h"
#include "ctrl_client.h"
#include "fof_queue.h"
#include "shmem.h"

void test_ctrl_client(void)
{
  ctrl_client_t* ctrl;

  int status;
  jack_nframes_t n ,m;
  fof_t fof;
  jack_nframes_t sample_rate;
  jack_nframes_t buffer_size;
  jack_time_t t0;
  setup_t setup;
  shmem_t *shmem;
  fof_queue_t* q;

  //TEST_ASSERT_EQUAL_UINT(0, sizeof(ctrl_client_t) % CACHE_LINE_SIZE);

  printf("fofs version: %s\n", fof_version());

  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 32;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;

  shmem = shmem_create(&setup, &status);
  TEST_ASSERT_NOT_NULL(shmem);

  size_t slots_off;
  size_t fofs_off;
  size_t size; 
  size = shmem_layout(&setup, &slots_off, &fofs_off);
  printf("shmem: %p - %p\n", shmem, (char*)shmem + size);

  q = &(shmem->q);
  ctrl = ctrl_client_new(&setup, q, &status);
  TEST_ASSERT_NOT_NULL(ctrl);
  TEST_ASSERT_NOT_NULL(ctrl->q);

  sample_rate = jack_get_sample_rate(ctrl->j_client);
  buffer_size = jack_get_buffer_size(ctrl->j_client);
  setup.sample_rate = sample_rate;
  setup.buffer_size = buffer_size;

  fof_queue_init(q, &setup);

  ctrl->dsp[0] = dsp_client_new(&setup, 0, &status);
  TEST_ASSERT_NOT_NULL(ctrl->dsp);
  
  t0 = jack_frame_time(ctrl->j_client);
  
  // Run empty for 2 sec.
  ctrl_client_activate(ctrl);
  TEST_ASSERT_EQUAL_UINT64(0, ctrl->q->next_frame);
  usleep(100);
  n = jack_frame_time(ctrl->j_client);
  usleep(2000000 - 100);
  m = jack_frame_time(ctrl->j_client);
  TEST_ASSERT_INT_WITHIN(500, 96000, m - n);
  n = (2 * sample_rate / buffer_size + 1) * buffer_size;
  TEST_ASSERT_INT_WITHIN(500, n, ctrl->q->next_frame);

  // Add a fof
  n = (jack_frame_time(ctrl->j_client) - t0 + 64) * 1000000ULL / sample_rate;
  fof_default(&fof);
  fof.time_us = n;
  status = fof_queue_add(ctrl->q, n, fof.argv);
  sleep(5);
  
  ctrl_client_free(ctrl);
}
