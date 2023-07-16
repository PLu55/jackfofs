#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "jfofs.h"
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

void fofs_sleep(jfofs_time_t t)
{
  struct timespec ts;
  struct timespec tr;
  
  ts.tv_sec = t / 1000000UL;
  ts.tv_nsec = (t - ts.tv_sec) * 1000UL;
  nanosleep(&ts, &tr);
}

void dump_statistic(jfofs_t* jfofs)
{
  statistics_t* stats = &(jfofs->shmem->statistics);
  int sum = 0;
  for (int i = 0; i < stats->n_slots; i++)
    sum += stats->slot_cnt[i];
  printf("Dumping statistics:\n");
  printf("total: %d\n", sum +  stats->late_cnt + stats->excess_cnt);
  printf("excluding late and excess: %d\n", sum);
  printf("late: %d\n", stats->late_cnt );
  printf("excess: %d\n", stats->excess_cnt );
  for (int i = 0; i < stats->n_slots; i++)
    printf("slot[%d]: %d\n", i + 1, stats->slot_cnt[i]);
}

void test_api(void)
{
  jfofs_t* jfofs;
  int status;
  fof_t fof;
  shmem_t* shmem;
  setup_t setup;
  struct timespec ts;
  struct timespec tr;
  
  signal_tester_client_t* stc;

  atexit(exit_handler);
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 64;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;
  setup.fofs_trace_level = 0;

#if 0 
  cpid = fork();
  printf("cpid: %d\n", cpid);
  if ( cpid == 0)
  {
    setup_signal_handlers();
    mgr = manager_create(&setup, &status);
    sleep(-1);
    exit(0);
  }
  
  fofs_sleep(100000);
#endif
  
  jfofs = jfofs_new(&status);
  if (jfofs == NULL)
    exit(-1);
  TEST_ASSERT_NOT_NULL(jfofs);
  TEST_ASSERT_NOT_NULL_MESSAGE(jfofs->shmem, "shared memory not linked");

  stc = signal_tester_client_new(&status);
  signal_tester_client_set_nframes(stc, (uint64_t)(setup.sample_rate * 1.1));
  signal_tester_client_activate(stc);
  
  dump_statistic(jfofs);

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
    fofs_sleep(50000);
  }
  signal_tester_client_deactivate(stc);
  printf("min: %f max: %f RMS: %f\n", stc->min, stc->max, signal_tester_client_rms(stc));
  dump_statistic(jfofs);

  if (cpid)
    printf("kill(%d): %d\n", cpid, kill(cpid, SIGTERM));
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
