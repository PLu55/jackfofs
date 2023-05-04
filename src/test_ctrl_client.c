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

void test_ctrl_client(void)
{
  ctrl_client* ctrl;

  int status;
  jack_nframes_t n ,m;
  fof _fof;
  jack_nframes_t sample_rate;
  jack_nframes_t buffer_size;
  jack_time_t t0;
  setup _setup;

  TEST_ASSERT_EQUAL_UINT(0, sizeof(ctrl_client) % CACHE_LINE_SIZE);

  printf("fofs version: %s\n", fof_version());

  _setup.mode = FOF_MONO;
  _setup.n_clients = 1;
  _setup.n_preallocate_fofs = 1024;
  _setup.n_slots = 64;
  _setup.n_free_chunks = 128;
  _setup.chunk_size = 256;
  
  ctrl = ctrl_client_new(&_setup, &status);
  TEST_ASSERT_NOT_NULL(ctrl);
  TEST_ASSERT_NOT_NULL(ctrl->q);
  TEST_ASSERT_EQUAL_INT(_setup.n_slots, ctrl->q->n_slots);
  TEST_ASSERT_EQUAL_INT(_setup.n_free_chunks, ctrl->q->n_free_chunks);

  sample_rate = jack_get_sample_rate(ctrl->j_client);
  buffer_size = jack_get_buffer_size(ctrl->j_client);
  TEST_ASSERT_EQUAL_UINT64(sample_rate, ctrl->q->sample_rate);
  TEST_ASSERT_EQUAL_UINT32(buffer_size, ctrl->q->slot_size);

  ctrl->dsp[0] = dsp_client_new(&_setup, &status);
  TEST_ASSERT_NOT_NULL(ctrl->dsp);
  
  t0 = jack_frame_time(ctrl->j_client);
  
  // Run empty for 2 sec.
  ctrl_client_activate(ctrl);
  TEST_ASSERT_EQUAL_UINT64(0, ctrl->q->next_frame);
  n = jack_frame_time(ctrl->j_client);
  sleep(2);
  m = jack_frame_time(ctrl->j_client);
  TEST_ASSERT_INT_WITHIN(128, 96000, m - n);
  TEST_ASSERT_INT_WITHIN(128, 96000, ctrl->q->next_frame);

  // Add a fof
  n = (jack_frame_time(ctrl->j_client) - t0 + 64) * 1000000ULL / sample_rate;
  fof_default(&_fof);
  _fof.time_us = n;
  status = fof_queue_add(ctrl->q, &_fof);
  sleep(5);
  
  ctrl_client_free(ctrl);
}
