#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include <unistd.h>

#include "jfofs.h"
#include "manager_dummy.h"
#include "signal_tester_client.h"
#include "test_util.h"

void test_api(void)
{
  jfofs* _jfofs;
  manager_dummy* dmgr;
  int status;
  jack_nframes_t sample_rate;
  int n_chans;
  fof _fof;
  pthread_t tid;
  jack_time_t t0;
  
  dmgr = manager_dummy_new(&status);
  TEST_ASSERT_NOT_NULL(dmgr);
  
  tid = manager_dummy_start(dmgr);

  _jfofs = jfofs_new(&status);
  TEST_ASSERT_NOT_NULL(_jfofs);

  //mgr->ctrl->n = 0;
  //mgr->ctrl->m = 0;
  //t0 = jack_get_time();
  //uint64_t n =  jfofs_nframes_to_time_us(mgr->q->next_frame, mgr->q->sample_rate);

  //_fof.time_us = t0 + 100000;
  _fof.time_us = 0;
  fof_default(&_fof);


  
  jfofs_add(_jfofs, &_fof);
  sleep(1);
  printf("cnt: %d\n", manager_dummy_get_count(dmgr));
  exit(0);
  return;

  manager_dummy_free(dmgr); // <--- craches here!!!
  jfofs_free(_jfofs);
  return;
}
