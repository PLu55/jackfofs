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

int count_free_fofs(fof_queue_t* q);

void test_manager_with_setup(setup_t* setup, int n_fofs, signal_tester_client_t* stc);

void test_manager(void)
{
  signal_tester_client_t* stc;
  int status;
  jack_nframes_t sample_rate;
  setup_t setup;

  setup.mode = FOF_MONO;
  //setup.mode = FOF_STEREO;
  //setup.mode = FOF_AMB1D;
  setup.fofs_trace_level = 0;
  setup.n_clients = 6;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024 * 10;
  setup.n_slots = 32;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;
  
  stc = signal_tester_client_new(&status);
  TEST_ASSERT_NOT_NULL(stc);
  signal_tester_client_activate(stc);
  sample_rate = jack_get_sample_rate(stc->j_client);
  signal_tester_client_set_nframes(stc, (uint64_t)(sample_rate * 1.1));

  test_manager_with_setup(&setup, 4000, stc);
  
  printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));

  TEST_ASSERT_FLOAT_WITHIN(0.1f, -1.0f, stc->min);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, stc->max);
  TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.195585f, signal_tester_client_rms(stc));
  signal_tester_client_free(stc);
}

void test_manager_with_setup(setup_t* setup, int n_fofs, signal_tester_client_t* stc)
{
  manager_t* mgr;
  fof_t fof;
  int status;
  int n_chans;
  char msg[128];
  
  n_chans = fof_ModeToChannels(setup->mode);
  mgr = manager_create(setup, &status);

  //sprintf(msg, "status: %d", status);
  TEST_ASSERT_NOT_NULL(mgr);
  TEST_ASSERT_NOT_NULL_MESSAGE(mgr, msg);
  TEST_ASSERT_NOT_NULL(mgr->ctrl);
  
  for (int i = 0; i < setup->n_clients; i++)
  {
    TEST_ASSERT_NOT_NULL(mgr->dsp[i]);         /* TODO: should remove dsp from mgr */
    TEST_ASSERT_NOT_NULL(mgr->ctrl->dsp[i]);
  }
  TEST_ASSERT_NOT_NULL(mgr->mix);
 
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
  //jack_time_t t0 =jack_frame_time(mgr->ctrl->j_client);
  uint64_t n =  jfofs_nframes_to_time(mgr->q->next_frame, mgr->q->sample_rate);
  jfofs_time_t t_us;
  
  t_us = n + 100000;
  fof_default(&fof);
  for(int i = 0; i < n_fofs; i++)
    manager_add(mgr, t_us, fof.argv);
  
  sleep(2);
  printf("next_frame: %ld\n", mgr->q->next_frame);

  TEST_ASSERT_EQUAL_INT(setup->n_max_fofs, count_free_fofs(mgr->q));
  TEST_ASSERT_EQUAL_INT(0, manager_deactivate_clients(mgr));
  manager_free(mgr);
}

int count_free_fofs(fof_queue_t* q)
{
  int i = 0;
  fof_t* fof = q->free_fofs;
  
  while (fof)
  {
    i++;
    fof = fof->next;
  }
  return i;
}
