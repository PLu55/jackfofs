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
  FofMode mode = FOF_MONO;
  int nclients = 1;
  int n_fofs_per_client = 16;
  int n_preallocate_fofs = 1024;
  int status;
  jack_nframes_t n ,m;
  fof _fof;
  jack_nframes_t sample_rate;
  jack_nframes_t buf_size;
  jack_time_t t0;


  ctrl = ctrl_client_new(mode, nclients, n_fofs_per_client, n_preallocate_fofs,
			 &status);
  t0 = jack_frame_time(ctrl->j_client);
  sample_rate = jack_get_sample_rate(ctrl->j_client);
  buf_size = jack_get_buffer_size(ctrl->j_client);

  TEST_ASSERT_NOT_NULL(ctrl);
  TEST_ASSERT_NOT_NULL(ctrl->q);

  // Run empty for 2 sec.
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
