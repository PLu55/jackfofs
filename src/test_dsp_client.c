#include <unistd.h>
#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>

#include "dsp_client.h"
#include "signal_tester_client.h"
#include "test_util.h"

void test_dsp_client(void)
{
  dsp_client_t* dsp;
  signal_tester_client_t* stc;
  int status;
  fof_t fof;
  setup_t setup;

  setup.mode = FOF_MONO;
  setup.fofs_trace_level = 0;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 32;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;

  TEST_ASSERT_EQUAL_INT(0, sizeof(dsp_client_t) % CACHE_LINE_SIZE);
  dsp = dsp_client_new(&setup, &status);
  TEST_ASSERT_NOT_NULL(dsp);
  TEST_ASSERT_NOT_NULL(dsp->j_client);
  TEST_ASSERT_EQUAL_INT(1, dsp->n_chans);
  TEST_ASSERT_NOT_NULL(dsp->in_port);
  TEST_ASSERT_NOT_NULL(dsp->out_port[0]);

  stc = signal_tester_client_new(&status);
  signal_tester_client_set_nframes(stc, (uint64_t)(setup.sample_rate * 1.1));

  signal_tester_client_activate(stc);
  fof.time_us = fof_time(dsp->fof_bank);
  fof_default(&fof);
  dsp_client_add(dsp, &fof);
  dsp_client_activate(dsp);

  jack_connect(dsp->j_client,
	       jack_port_name(dsp->out_port[0]),
	       jack_port_name(stc->in_port));

  TEST_ASSERT_TRUE(jack_port_connected_to(dsp->out_port[0],
					  jack_port_name(stc->in_port)));

  sleep(2);

  jack_disconnect(dsp->j_client,
		  jack_port_name(dsp->out_port[0]),
		  jack_port_name(stc->in_port));
  signal_tester_client_deactivate(stc);
  dsp_client_deactivate(dsp);

  //printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, -1.0f, stc->min);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, stc->max);
  TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.195585f, signal_tester_client_rms(stc));

  signal_tester_client_free(stc);
  dsp_client_free(dsp);
}
