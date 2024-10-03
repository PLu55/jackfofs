#include "jfofs.h"
#include "jfofs_private.h"
#include "shmem.h"
#include "test_util.h"

char* fof_arg_name[] = { "ampl", "freq","gliss", "phi", "beta",
  "alpha", "amin", "cutoff", "pan1", "pan2", "pan3" };

void fof_default(fof_t* fof)
{
  fof->argv[FOF_ARG_ampl]   =     1.0f;  
  fof->argv[FOF_ARG_freq]   =   100.0f;
  fof->argv[FOF_ARG_gliss]  =    0.0f;
  fof->argv[FOF_ARG_phi]    =     0.0f;
  fof->argv[FOF_ARG_beta]   =     0.1f;
  fof->argv[FOF_ARG_alpha]  =  13.816f;  /* one second duration */
  fof->argv[FOF_ARG_amin]   =   0.001f;
  fof->argv[FOF_ARG_cutoff] =   0.002f;
  fof->argv[FOF_ARG_pan1]   =     0.0f;
  fof->argv[FOF_ARG_pan2]   =     0.0f;
  fof->argv[FOF_ARG_pan3]   =     0.0f;
}

void fof_print(fof_t* fof)
{
  printf("fof @%p:\n", fof);
  printf("     time: %8ld\n", fof->time_us);
  for (int i = 0; i < FOF_NUMARGS; i++)
    printf("   %6s: %8.4f\n", fof_arg_name[i], fof->argv[i]);
  printf("\n");
}

int fof_equal(fof_t* fof1, fof_t* fof2)
{
  int r = 1;
  for (int i = 0; i < FOF_NUMARGS; i++)
  {
    r = r && fof1->argv[i] == fof2->argv[i];
  }
  
  return
    r && fof1->time_us == fof2->time_us;
}

void jfofs_sleep(jfofs_time_t t)
{
  struct timespec ts;
  struct timespec tr;
  
  ts.tv_sec = t / 1000000UL;
  ts.tv_nsec = (t - ts.tv_sec) * 1000UL;
  nanosleep(&ts, &tr);
}

void dump_setup(jfofs_t* jfofs)
{
  setup_t* shm_setup  = jfofs_get_setup(jfofs);

  printf("Dumping setup:\n");
  printf("  mode: %d\n", shm_setup->mode);
  printf("  trace_level: %d\n", shm_setup->fofs_trace_level);
  printf("  n_clients: %d\n", shm_setup->n_clients);
  printf("  n_preallocate: %d\n", shm_setup->n_preallocate_fofs);
  printf("  n_max_fofs: %d\n",  shm_setup->n_max_fofs);
  printf("  n_slots: %d\n",  shm_setup->n_slots);
  printf("  sample_rate: %d\n", shm_setup->sample_rate);
  printf("  buffer_size: %d\n", shm_setup->max_buffer_size);
  printf("  xrun_limit: %d\n", shm_setup->xrun_limit);
}
