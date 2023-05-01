#include <unistd.h>
#include <stdio.h>
#include <unity/unity.h>
#include <jack/jack.h>
#include <fofs.h>

#include "jfofs.h"
#include "test_util.h"
#include "ctrl_client.h"
#include "fof_queue.h"

void test_ctrl_client(void)
{
  ctrl_client* ctrl;

  int status;
  jack_nframes_t n ,m;
  fof _fof;
  jack_nframes_t sample_rate;
  jack_time_t t0;
  setup _setup;

  _setup.mode = FOF_MONO;
  _setup.n_clients = 1;
  _setup.n_preallocate_fofs = 1024;
  _setup.n_slots = 64;
  _setup.n_free_chunks = 128;
  _setup.chunk_size = 256;

  ctrl = ctrl_client_new(&_setup, &status);

  t0 = jack_frame_time(ctrl->j_client);
  sample_rate = jack_get_sample_rate(ctrl->j_client);

  TEST_ASSERT_NOT_NULL(ctrl);
  TEST_ASSERT_NOT_NULL(ctrl->q);

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
  
}
