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
  ctrl_client_t *ctrl;
  dsp_client_t *dsp;

  int status;
  jack_nframes_t n, m;
  fof_t fof;
  jack_time_t t0;
  setup_t setup;
  shmem_t *shmem;
  fof_queue_t *q;

  // TEST_ASSERT_EQUAL_UINT(0, sizeof(ctrl_client_t) % CACHE_LINE_SIZE);

#ifdef VERBOSE_ENABLE
  printf("fofs version: %s\n", fof_version());
#endif

  setup.fofs_trace_level = 0;
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 32;
  setup.sample_rate = 0;     // Not known yet, normally set by manager
  setup.max_buffer_size = 0; // Not known yet, normally set by manager

  // Check if Jack is running
  // TEST_ASSERT_GREATER_THAN_INT(44100, setup.sample_rate);

  shmem = shmem_create(&setup, &status);
  TEST_ASSERT_NOT_NULL(shmem);

  size_t slots_off;
  size_t fofs_off;
  size_t size;
  size = shmem_layout(&setup, &slots_off, &fofs_off);
  (void)size;

#ifdef VERBOSE_ENABLE
  printf("shmem: %p - %p\n", (void *)shmem, (void *)((char *)shmem + size));
#endif

  q = &(shmem->q);
  ctrl = ctrl_client_new(&setup, q, &status);
  TEST_ASSERT_NOT_NULL(ctrl);
  TEST_ASSERT_NOT_NULL(ctrl->q);

  setup.sample_rate = jack_get_sample_rate(ctrl->j_client);
  setup.max_buffer_size = jack_get_buffer_size(ctrl->j_client);
  printf("sample_rate: %d buffer_size: %d\n", setup.sample_rate, setup.max_buffer_size);

  fof_queue_init(q, &setup);
  TEST_ASSERT_EQUAL_UINT64(0, ctrl->q->next_frame);

  ctrl->dsp[0] = dsp_client_new(&setup, 0, &status);
  TEST_ASSERT_NOT_NULL(ctrl->dsp);
  dsp = ctrl->dsp[0];
  TEST_ASSERT_NOT_NULL(dsp);

  t0 = jack_frame_time(ctrl->j_client);

  // Run empty for 2 sec.
  ctrl_client_activate(ctrl);

  // Wait until client is running
  for (int i = 0; i < 100; i++)
  {
    if (ctrl->q->next_frame > 0)
      break;
    usleep(1000);
  }

  n = jack_frame_time(ctrl->j_client);
  usleep(2000000 - 100);
  m = jack_frame_time(ctrl->j_client);

#ifdef VERBOSE_ENABLE
  printf("n: %u, m: %u m-n: %u\n", n, m, m - n);
#endif

  TEST_ASSERT_INT_WITHIN(10, 96000, m - n);

  uint64_t expected_frames = 2ULL * (uint64_t)setup.sample_rate;
  uint64_t tolerance = 2ULL * (uint64_t)setup.max_buffer_size;
  TEST_ASSERT_UINT64_WITHIN(tolerance, expected_frames, ctrl->q->next_frame);

  // Add a fof
  n = (jack_frame_time(ctrl->j_client) - t0 + 64) * 1000000ULL / setup.sample_rate;
  fof_default(&fof);
  fof.time_us = n;
  status = fof_queue_add(ctrl->q, n, fof.argv);
  sleep(5);

  dsp_client_free(dsp);
  ctrl_client_free(ctrl);
  shmem_unmap(shmem);
  shmem_unlink(shmem);
}
