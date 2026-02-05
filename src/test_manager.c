#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include <unistd.h>
#include <string.h>

#include "signal_tester_client.h"
#include "manager.h"
#include "test_util.h"
#include "fof_queue.h"
#include "jfofs.h"

int count_free_fofs(fof_queue_t *q);

void test_manager_with_setup(setup_t *setup, int n_fofs, signal_tester_client_t *stc);

void test_manager(void)
{
  signal_tester_client_t *stc;
  int status;
  // jack_nframes_t sample_rate;
  setup_t setup;
  int n_fofs = 100;
  setup.mode = FOF_MONO;
  // setup.mode = FOF_STEREO;
  // setup.mode = FOF_AMB1D;
  setup.fofs_trace_level = 40;
  setup.n_clients = 6;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024 * 10;
  setup.n_slots = 32;
  setup.sample_rate = 48000;
  setup.max_buffer_size = 256;

  stc = signal_tester_client_new(&status);
  TEST_ASSERT_NOT_NULL(stc);
  setup.sample_rate = jack_get_sample_rate(stc->j_client);
  setup.max_buffer_size = jack_get_buffer_size(stc->j_client);
  signal_tester_client_set_nframes(stc, (uint64_t)(setup.sample_rate * 1.1));
  signal_tester_client_activate(stc);

  test_manager_with_setup(&setup, n_fofs, stc);

  float min = stc->min;
  float max = stc->max;
  float rms = signal_tester_client_rms(stc);

  printf("min: %f max: %f RMS: %f\n", min, max, rms);

  /* Unity assertions abort the test via longjmp; free resources first. */
  signal_tester_client_free(stc);

  const float per_fof_ampl = 0.0001f;
  const float expected_peak = (float)n_fofs * per_fof_ampl;
  const float abs_min = (min < 0.0f) ? -min : min;
  const float abs_max = (max < 0.0f) ? -max : max;
  const float abs_peak = (abs_min > abs_max) ? abs_min : abs_max;

  /* This test uses real-time JACK scheduling; exact peak can vary depending on
   * graph timing and overlap. Assert sane bounds to avoid flakiness.
   */
  TEST_ASSERT_TRUE_MESSAGE(min < 0.0f, "expected negative min");
  TEST_ASSERT_TRUE_MESSAGE(max > 0.0f, "expected positive max");
  TEST_ASSERT_TRUE_MESSAGE(abs_peak > 0.1f * expected_peak, "peak too small (no/low signal?)");
  TEST_ASSERT_TRUE_MESSAGE(abs_peak < 2.0f * expected_peak, "peak too large (unexpected gain?)");

  TEST_ASSERT_TRUE_MESSAGE(rms > 0.0f, "expected nonzero RMS");
  TEST_ASSERT_TRUE_MESSAGE(rms < abs_peak, "expected RMS < peak");
  const float ratio = rms / abs_peak;
  TEST_ASSERT_TRUE_MESSAGE(ratio > 0.05f && ratio < 0.40f, "unexpected RMS/peak ratio");
}

void test_manager_with_setup(setup_t *setup, int n_fofs, signal_tester_client_t *stc)
{
  manager_t *mgr;
  fof_t fof;
  int status;
  int n_chans;
  char msg[128];

  n_chans = fof_ModeToChannels(setup->mode);
  mgr = manager_create(setup, &status);

  // sprintf(msg, "status: %d", status);
  // printf( "status: %s\n", strerror(status));
  // perror("hepp");
  // char *strerror(17);
  TEST_ASSERT_NOT_NULL(mgr);
  TEST_ASSERT_NOT_NULL_MESSAGE(mgr, msg);
  TEST_ASSERT_NOT_NULL(mgr->ctrl);

  for (int i = 0; i < setup->n_clients; i++)
  {
    TEST_ASSERT_NOT_NULL(mgr->dsp[i]); /* TODO: should remove dsp from mgr */
    TEST_ASSERT_NOT_NULL(mgr->ctrl->dsp[i]);
  }
  TEST_ASSERT_NOT_NULL(mgr->mix);

  usleep(100000);

  for (int i = 0; i < setup->n_clients; i++)
  {
    TEST_ASSERT_TRUE(jack_port_connected_to(mgr->ctrl->port,
                                            jack_port_name(mgr->dsp[i]->in_port)));
    for (int j = 0; j < n_chans; j++)
    {
      TEST_ASSERT_TRUE(jack_port_connected_to(mgr->dsp[i]->out_port[j],
                                              jack_port_name(mgr->mix->in_port[j])));
    }
  }

  for (int i = 0; i < n_chans; i++)
  {
    jack_connect(mgr->ctrl->j_client,
                 jack_port_name(mgr->mix->out_port[i]),
                 jack_port_name(stc->in_port));
  }

  mgr->ctrl->n = 0;
  mgr->ctrl->m = 0;
  // jack_time_t t0 =jack_frame_time(mgr->ctrl->j_client);
  uint64_t next_frame = __atomic_load_n(&(mgr->q->next_frame), __ATOMIC_ACQUIRE);
  uint64_t n = jfofs_nframes_to_time(next_frame, mgr->q->sample_rate);
  jfofs_time_t t_us;

  printf("next_frame: %ld n: %ld\n", next_frame, n);
  t_us = n + 100000;

  fof_default(&fof);
  /* The manager test sums many simultaneous fofs; keep per-fof amplitude small
   * to avoid clipping/excessive levels and keep the expected RMS stable.
   */
  fof.argv[FOF_ARG_ampl] = 0.0001f;

  for (int i = 0; i < n_fofs; i++)
  {
    status = manager_add(mgr, t_us, fof.argv);
    TEST_ASSERT_EQUAL_INT_MESSAGE(JFOFS_SUCCESS, status, "manager_add failed (likely late/excess FOF scheduling)");
  }

  usleep(2000000);
  printf("next_frame: %ld\n", mgr->q->next_frame);

  // TEST_ASSERT_EQUAL_INT(setup->n_max_fofs, count_free_fofs(mgr->q));
  // TEST_ASSERT_EQUAL_INT(0, manager_deactivate_clients(mgr));
  manager_free(mgr);
}

int count_free_fofs(fof_queue_t *q)
{
  int i = 0;
  fof_t *fof = q->free_fofs;

  while (fof)
  {
    i++;
    fof = fof->next;
  }
  return i;
}
