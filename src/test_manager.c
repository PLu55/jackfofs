#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include<unistd.h>

#include "signal_tester_client.h"
#include "manager.h"
#include "test_util.h"
#include "fof_queue.h"
#include "jfofs.h"

void test_manager(void)
{
  manager* mgr;
  signal_tester_client* stc;
  fof _fof;
  int status;

  mgr = manager_new(&status);
  TEST_ASSERT_NOT_NULL(mgr);
  TEST_ASSERT_NOT_NULL(mgr->ctrl);
  TEST_ASSERT_NOT_NULL(mgr->dsp[0]);
  //TEST_ASSERT_NOT_NULL(mgr->mix);
  TEST_ASSERT_EQUAL_INT(0, manager_activate_clients(mgr));

  TEST_ASSERT_EQUAL_INT(0, manager_connect_clients(mgr));
  TEST_ASSERT_TRUE(jack_port_connected_to(mgr->ctrl->port,
					  jack_port_name(mgr->dsp[0]->in_port)));

  stc = signal_tester_client_new(&status);
  stc->m = (uint64_t)(48000.0 * 1.1);
  signal_tester_client_activate(stc);
  jack_connect(mgr->dsp[0]->j_client,
	       jack_port_name(mgr->dsp[0]->out_port[0]),
	       jack_port_name(stc->in_port));
    
    //"jfofs_dsp:out_1", "signal_tester:in");

  mgr->ctrl->n = 0;
  mgr->ctrl->m = 0;
  jack_time_t t0 =jack_frame_time(mgr->ctrl->j_client);
  uint64_t n =  jfofs_nframes_to_time_us(mgr->q->next_frame, mgr->q->sample_rate);
  
  _fof.time_us = n + 100000;
  fof_default(&_fof);
  manager_add(mgr, &_fof);
  printf("slot_18: %p cnt: %d\n", mgr->q->slot[18], mgr->q->slot[18]->count);
  print_fof(&(mgr->q->slot[18]->fof[0]));
  
  sleep(5);
  printf("slot_18: %p\n", mgr->q->slot[18]);
  printf("n: %d m: %d\n", mgr->ctrl->n, mgr->ctrl->m);
  printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));
  signal_tester_client_free(stc);
  manager_free(mgr);
}

