#include <unistd.h>
#include <stdio.h>
#include "unity/unity.h"

#include "dsp_client.h"
#include "signal_tester_client.h"
#include "test_util.h"

void test_dsp_client(void)
{
  FofMode mode = FOF_MONO;
  int n_fofs_per_client = 1024;
  dsp_client* dsp;
  signal_tester_client* stc;
  int status;
  fof fof;
  
  dsp = dsp_client_new(mode, n_fofs_per_client, &status);
  TEST_ASSERT_NOT_NULL(dsp);
  TEST_ASSERT_NOT_NULL(dsp->j_client);
  TEST_ASSERT_EQUAL_INT(1, dsp->n_chans);
  TEST_ASSERT_NOT_NULL(dsp->in_port);
  TEST_ASSERT_NOT_NULL(dsp->out_port[0]);

  stc = signal_tester_client_new(&status);
  stc->m = (uint64_t)(48000.0 * 1.1);
  signal_tester_client_activate(stc);
  fof.time = fof_time(dsp->fof_bank);
  fof_default(&fof);
  dsp_client_add(dsp, &fof);
  dsp_client_activate(dsp);
  jack_connect(dsp->j_client, "jfofs_dsp:out_1", "signal_tester:in");

  sleep(2);
  
  jack_disconnect(dsp->j_client, "jfofs_dsp:out_1", "signal_tester:in");
  dsp_client_deactivate(dsp);
  signal_tester_client_deactivate(stc);
  printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, -1.0f, stc->min);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, stc->max);
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.195585f, signal_tester_client_rms(stc));
}
