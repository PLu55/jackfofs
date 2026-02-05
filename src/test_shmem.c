#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "jfofs.h"
#include "config.h"
// #include "manager_dummy.h"
#include "signal_tester_client.h"
#include "test_util.h"
#include "manager.h"
#include "shmem.h"
#include "api.h"

static pid_t cpid = 0;

#if 0
static void termination_handler (int signum)
{
  printf("terminating: %d\n", signum);
  exit(-1);
}

static void exit_handler(void)
{
  if (cpid)
    kill(cpid, SIGTERM);
}

static void setup_signal_handlers()
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
#endif

static void set_fof(fof_t *fof)
{
  fof_default(fof);
  fof->argv[FOF_ARG_freq] = 1000.0f;
  fof->argv[FOF_ARG_ampl] = 0.01000f;
  fof->argv[FOF_ARG_alpha] = 138.2f; /* 0.1s alpha = log(1e-6)/-t60 */
  fof->argv[FOF_ARG_beta] = 0.01f;
}

static void fof_bursts(jfofs_t *jfofs, fof_t *fof, int n_fofs, int n_bursts,
                       uint64_t dt_fof_us, uint64_t dt_burst_u)
{
  (void)dt_fof_us;
  uint64_t t;
  int status;

  for (int j = 0; j < n_bursts; j++)
  {
    for (int i = 0; i < n_fofs; i++)
    {
      int l = rand() % 1000;
      for (int k = 0; k < l; k++)
        sin((double)rand() / (double)RAND_MAX);
      t = jfofs_get_time(jfofs) + 30000UL;
      status = jfofs_add(jfofs, t,
                         fof->argv[FOF_ARG_ampl],
                         fof->argv[FOF_ARG_freq],
                         fof->argv[FOF_ARG_gliss],
                         fof->argv[FOF_ARG_phi],
                         fof->argv[FOF_ARG_beta],
                         fof->argv[FOF_ARG_alpha],
                         fof->argv[FOF_ARG_amin],
                         fof->argv[FOF_ARG_cutoff],
                         fof->argv[FOF_ARG_pan1],
                         fof->argv[FOF_ARG_pan2],
                         fof->argv[FOF_ARG_pan3]);
      if (status && status != JFOFS_FOF_LATE_WARNING)
        printf("jfofs_add: error: %d\n", status);
      // jfofs_sleep(dt_fof_us);
    }
    // printf("pid: %d burst: %d\n", cpid, j);
    jfofs_sleep(dt_burst_u);
  }
}

/* testing multi client access */
/* it seames like problems can be generated when the fof freelist is empty. */

void test_shmem(void)
{
  jfofs_t *jfofs;
  int status;
  fof_t fof;
  int n_fofs = 10000;
  int n_bursts = 1000;
  uint64_t dt_fofs_us = 1;
  uint64_t dt_burst_us = 1000;

#ifndef DEBUG_ENABLE
  printf("Debug mode must be enabled to run this test.\n");
  exit(1);
#endif

#if 0
  setup_t setup;

  atexit(exit_handler);
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 1024;
  setup.n_max_fofs = 1024;
  setup.n_slots = 64;
  setup.sample_rate = 48000;
  setup.buffer_size = 256;
  setup.fofs_trace_level = 0;
#endif

#if 1 // 0
  /* parallel jfofs client */
  cpid = fork();
  printf("cpid: %d\n", cpid);
  if (cpid == 0)
  {
    printf("Client 1\n");
    // setup_signal_handlers();
    // mgr = manager_create(&setup, &status);
    jfofs = jfofs_new(&status, NULL);
    if (jfofs == NULL)
      exit(-1);
    TEST_ASSERT_NOT_NULL(jfofs);
    TEST_ASSERT_NOT_NULL_MESSAGE(jfofs->shmem, "shared memory not linked");
    STATISTICS_INIT;

    jfofs_sleep(100000);
    set_fof(&fof);
    fof_bursts(jfofs, &fof, n_fofs, n_bursts, dt_fofs_us, dt_burst_us);
    printf("Client 1 is done!\n");
    jfofs_free(jfofs);
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

  // dump_setup(jfofs);
  printf("sample rate: %d\n", jfofs_sample_rate(jfofs));
  printf("buffer size: %d\n", jfofs_buffer_size(jfofs));
  // jfofs->shmem->setup
  set_fof(&fof);
  fof_bursts(jfofs, &fof, n_fofs, n_bursts, dt_fofs_us, dt_burst_us);

  if (0 && cpid)
    printf("kill(%d): %d\n", cpid, kill(cpid, SIGTERM));

  jfofs_sleep(1000000);

  DUMP_STATISTICS();

  int cnt = 0;
  int istat;
  istat = CHECK_FREE_LIST(&(jfofs->shmem->q), &cnt, 1);
  printf("Fof free list integrity:\n   status: %d(zero is good)\n   cnt: %d/%d\n",
         istat, cnt, jfofs->shmem->setup.n_max_fofs);
  TEST_ASSERT_EQUAL_INT(0, istat);
  TEST_ASSERT_EQUAL_INT(jfofs->shmem->setup.n_max_fofs, cnt);

  jfofs_free(jfofs);
  printf("Client 2 is done\n");
  exit(0);
}
