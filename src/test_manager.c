#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include <unistd.h>

#include "signal_tester_client.h"
#include "manager.h"
#include "test_util.h"
#include "fof_queue.h"
#include "jfofs.h"

void test_manager_with_setup(setup* _setup, signal_tester_client* stc);

void test_manager(void)
{
  signal_tester_client* stc;
  int status;
  jack_nframes_t sample_rate;
  setup _setup;

  //_setup.mode = FOF_MONO;
  //_setup.mode = FOF_STEREO;
  _setup.mode = FOF_AMB1D;
  _setup.n_clients = 6;
  _setup.n_preallocate_fofs = 1024;
  _setup.n_slots = 64;
  _setup.n_free_chunks = 128;
  _setup.chunk_size = 256;
  
  stc = signal_tester_client_new(&status);
  TEST_ASSERT_NOT_NULL(stc);
  signal_tester_client_activate(stc);
  sample_rate = jack_get_sample_rate(stc->j_client);
  stc->m = (uint64_t)(sample_rate * 1.1);

  test_manager_with_setup(&_setup, stc);
  
  printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));

  TEST_ASSERT_FLOAT_WITHIN(0.1f, -1.0f, stc->min);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, stc->max);
  TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.195585f, signal_tester_client_rms(stc));
  signal_tester_client_free(stc);
}

void test_manager_with_setup(setup* _setup, signal_tester_client* stc)
{
  manager* mgr;
  fof _fof;
  int status;
  int n_chans;
  
  n_chans = fof_ModeToChannels(_setup->mode);
  
  mgr = manager_new(_setup, &status);
  TEST_ASSERT_NOT_NULL(mgr);
  TEST_ASSERT_NOT_NULL(mgr->ctrl);
  
  for (int i = 0; i < _setup->n_clients; i++)
  {
    TEST_ASSERT_NOT_NULL(mgr->dsp[i]);         /* TODO: should remove dsp from mgr */
    TEST_ASSERT_NOT_NULL(mgr->ctrl->dsp[i]);
  }
  TEST_ASSERT_NOT_NULL(mgr->mix);
  TEST_ASSERT_EQUAL_INT(0, manager_activate_clients(mgr));
  TEST_ASSERT_EQUAL_INT(0, manager_connect_clients(mgr));

  for (int i = 0; i < _setup->n_clients; i++)
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
  jack_time_t t0 =jack_frame_time(mgr->ctrl->j_client);
  uint64_t n =  jfofs_nframes_to_time_us(mgr->q->next_frame, mgr->q->sample_rate);

  _fof.time_us = n + 100000;
  fof_default(&_fof);
  for(int i = 0; i < 1000; i++)
    manager_add(mgr, &_fof);
  
  sleep(2);

  TEST_ASSERT_EQUAL_INT(0, manager_deactivate_clients(mgr));
  manager_free(mgr);
}
