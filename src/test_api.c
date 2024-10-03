#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "jfofs.h"
#include "config.h"
//#include "manager_dummy.h"
#include "signal_tester_client.h"
#include "test_util.h"
#include "manager.h"
#include "shmem.h"
#include "api.h"

static manager_t* mgr = NULL;
static jfofs_t* jfofs = NULL;
static pid_t cpid = 0;

static void exit_handler(void)
{
  if (mgr != NULL)
  {
    manager_deactivate_clients(mgr);
    manager_free(mgr);
  }
  if (jfofs != NULL)
  {
    jfofs_free(jfofs);
  }
  if (cpid)
    kill(cpid, SIGTERM);
}

static void termination_handler (int signum)
{
  printf("terminating: %d\n", signum);
  exit(-1);
}

void setup_signal_handlers()
{
  struct sigaction new_action, old_action;

  new_action.sa_handler = termination_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGHUP, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGHUP, &new_action, NULL);
  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTERM, &new_action, NULL);
}

void test_api(void)
{
  jfofs_t* jfofs;
  int status;
  fof_t fof;
  setup_t setup;
  setup_t* shm_setup;
  
  signal_tester_client_t* stc;

  atexit(exit_handler);
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 64;
  setup.sample_rate = 48000;
  setup.max_buffer_size = 256;
  setup.fofs_trace_level = 0;

#if 0
  /* Starting jfofs demon */
  /* could not mapp shmem in jfofs_new, address is occupide */   
  cpid = fork();
  printf("cpid: %d\n", cpid);
  if ( cpid == 0)
  {
    setup_signal_handlers();
    mgr = manager_create(&setup, &status);
    sleep(-1);
    exit(0);
  }
  
  jfofs_sleep(100000);
  sleep(-1);
#endif

#if 1 //0
  /* parallel jfofs client */
  cpid = fork();
  printf("cpid: %d\n", cpid);
  if ( cpid == 0)
  {
     printf("Client 1\n");
    // setup_signal_handlers();
    // mgr = manager_create(&setup, &status);
    jfofs = jfofs_new(&status, NULL);
    sleep(-1);
    exit(0);
  }
  
  jfofs_sleep(100000);
  printf("Client 2\n");
#endif
 
  jfofs = jfofs_new(&status, NULL);
  if (jfofs == NULL)
    exit(-1);
  TEST_ASSERT_NOT_NULL(jfofs);
  TEST_ASSERT_NOT_NULL_MESSAGE(jfofs->shmem, "shared memory not linked");

  //setup.sample_rate = jfofs_sample_rate(jfofs);
  printf("Sample rate: %d\n", setup.sample_rate);
  shm_setup = jfofs_get_setup(jfofs);
  TEST_ASSERT_NOT_NULL(shm_setup);
  printf("shm_setup: %p\n", shm_setup);
  printf("Sample rate: %d\n", shm_setup->sample_rate);
  dump_setup(jfofs);
  printf("Reference_cnt: %d\n", jfofs->shmem->reference_cnt);
  TEST_ASSERT_EQUAL_INT(1, jfofs->shmem->reference_cnt);
  
  stc = signal_tester_client_new(&status);
  signal_tester_client_set_nframes(stc, (uint64_t)(setup.sample_rate * 1.1));
  signal_tester_client_activate(stc);
  
  DUMP_STATISTICS();

  jack_connect(stc->j_client,"jfofs_mix:out_1",
	       jack_port_name(stc->in_port));
    
  //TEST_ASSERT_TRUE(&(jfofs->q) == &(shmem->q));

  fof_default(&fof);
  
  TEST_ASSERT_NULL(jfofs->shmem->q.slot[0]);
  uint64_t f = jfofs->shmem->q.next_frame;
  uint64_t t = f * 1000000 / setup.sample_rate;
  fof.argv[FOF_ARG_freq]   =   1000.0f;
  fof.argv[FOF_ARG_ampl]   =   0.01000f;
  fof.argv[FOF_ARG_alpha]  =  100.0f;
  fof.argv[FOF_ARG_beta]  =  0.01f;

  for (int i = 0; i < 20; i++)
  {
    t = jfofs_get_time(jfofs) + 5000UL;
    fof.argv[FOF_ARG_freq]   =   1000.0f;
    jfofs_add(jfofs, t + 10000UL, fof.argv);

    /*
    fof.argv[FOF_ARG_freq]   =   2000.0f;
    fof.argv[FOF_ARG_ampl]   =   0.005000f;
    jfofs_add(jfofs, t + 10000UL, fof.argv);
    fof.argv[FOF_ARG_freq]   =   3000.0f;
     fof.argv[FOF_ARG_ampl]   =   0.001000f;
    jfofs_add(jfofs, t + 10000UL, fof.argv);
    */
    //TEST_ASSERT_NOT_NULL(jfofs->q.slot[0]);
#if 0
    for (int i = 0; i < setup.n_slots; i++)
    {
	if (jfofs->shmem->q.slot[i] != NULL)
	  printf("slot: %d\n", i);
    }
#endif
    jfofs_sleep(50000);
  }
  signal_tester_client_deactivate(stc);
  printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));
  DUMP_STATISTICS();

  if (cpid)
    printf("kill(%d): %d\n", cpid, kill(cpid, SIGTERM));

  jfofs_free(jfofs);
  // can't do this here TEST_ASSERT_EQUAL_INT(0, jfofs->shmem->reference_cnt);

  exit(0);
  
#if 0
  jfofs* _jfofs;
  manager_dummy* dmgr;
  jack_nframes_t sample_rate;
  int n_chans;
  fof _fof;
  pthread_t tid;
  jack_time_t t0;
  
  dmgr = manager_dummy_new(&status);
  TEST_ASSERT_NOT_NULL(dmgr);
  
  tid = manager_dummy_start(dmgr);


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
#endif
}
