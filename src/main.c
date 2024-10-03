#include <stdlib.h>
#include <unistd.h>
#include <argp.h>
#include <signal.h>

#include "config.h"
#include "jfofs_private.h"
#include "manager.h"
#include "pipewire_query.h"

const char *argp_program_version = PROJECT_NAME_VER;

static char doc[] =
  "jfofs is a jack client program that is a parallel fof synthesizer";

static struct argp_option options[] = {
  {"verbose",     'v', 0,       0, "Produce verbose output" },
  {"mode",        'm', "mode",  0,
   "Fof mode: 1: mono, 2: stereo 3: quad: 4: ambiO1 5: ambi01D, default is 1" },
  {"n_clients",   'n', "n",     0, "number of parallel clients, default is 1" },
  {"n_fofs",      'p', "n",     0, "number of preallocated fofs in libfofs, default is 10240" },
  {"n_slots",     's', "n",     0, "number of slots in circular queue, default is 32" },
  {"n_max_fofs",  'q', "n",     0, "maximum number of fofs in queue, default is 1024" },
  {"trace_level", 't', "trace", 0, "sets the trace level, default is 0"},
  {"xrun_limit",  'x', "n",     0, "terminates the server if more xruns then the limit, 0 is no limit" },
  { 0 }
};

struct arguments
{
  int verbose;
  int mode;
  int n_clients;
  int n_fofs;
  int n_slots;
  int n_max_fofs;
  int trace_level;
  int xrun_limit;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'm':
      arguments->mode = atoi(arg);
      break;
    case 'n':
      arguments->n_clients = atoi(arg);
      break;
    case 'p':
      arguments->n_fofs = atoi(arg);
      break;
    case 's':
      arguments->n_slots = atoi(arg);
      break;
    case 'q':
      arguments->n_max_fofs = atoi(arg);
      break;
    case 't':
      arguments->trace_level = atoi(arg);
      break;
    case 'x':
      arguments->xrun_limit = atoi(arg);
      break;

#if 0
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
        argp_usage (state);

      arguments->args[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 0)
        /* Not enough arguments. */
        argp_usage (state);
      break;
#endif
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };

static manager_t* mgr = NULL;

static void exit_handler(int status)
{
  fprintf(stderr, "exit_handler!\n");
  if (mgr != NULL)
  {
    DUMP_STATISTICS();

#ifdef DEBUG_ENABLE
    int cnt = 0;
    int istat;
    istat = CHECK_FREE_LIST(&(mgr->shmem->q), &cnt, 1);
    printf("Fof free list integrity:\n   status: %d\n   fof cnt: %d/%d\n",
	   istat, cnt, mgr->shmem->setup.n_max_fofs);
#endif
    manager_deactivate_clients(mgr);
    manager_free(mgr);
  }

  exit(status != 0);
}

static void termination_handler (int signum)
{
  fprintf(stderr, "Terminating!\n");
#ifdef STATISTICS_ENABLE
  
  if (signum == SIGTSTP)
  {
    statistics_t* stats = &(mgr->shmem->statistics);
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
#endif
  
#ifdef DEBUG_ENABLE
  if (signum == SIGTSTP)
  {
    int i = 0;
    int cnt = 0;
    i = check_free_list(mgr->q, &cnt, 1);
    printf("Debug: free_list integrity check(zero is good): %d free count: %d\n",
	      i, cnt);
  }
#endif

  exit_handler(1);
}

int main (int argc, char **argv)
{
  struct arguments arguments;
  setup_t setup;
  int status;
  struct sigaction new_action, old_action;

  mgr = NULL;

  new_action.sa_handler = termination_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = 0;

  /* ^\ generats a core dump */
  sigaction (SIGINT, NULL, &old_action);      /* ^C */
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGHUP, NULL, &old_action);       
  if (old_action.sa_handler != SIG_IGN)       /* doen't work */
    sigaction (SIGHUP, &new_action, NULL);
  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTERM, &new_action, NULL);
  sigaction (SIGTSTP, NULL, &old_action);     
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTSTP, &new_action, NULL);   /* ^Z */

  arguments.verbose = 0;
  arguments.mode = 1;
  arguments.n_clients = 1;
  arguments.n_fofs = 10 * 1024;
  arguments.n_max_fofs = 1024;
  arguments.n_slots = 32;
  arguments.trace_level = 0;
  arguments.xrun_limit = 0;
  
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  setup.mode = arguments.mode;
  setup.n_clients = arguments.n_clients;
  setup.n_preallocate_fofs =  arguments.n_fofs;
  setup.n_slots =  arguments.n_slots;
  setup.n_max_fofs = arguments.n_max_fofs;
  setup.fofs_trace_level = arguments.trace_level;
  setup.xrun_limit = arguments.xrun_limit;
  setup.verbose = arguments.verbose;

#ifdef HAS_PIPEWIRE
  char* rate = pipewire_query("default.clock.rate");
  char* buffer = pipewire_query("default.clock.max-quantum");
  if (rate == NULL || buffer == NULL)
  {
    printf("Couldn't get the sample rate or buffer size from pipewire!\n");
    exit_handler(1);
  }
  setup.sample_rate = atoi(pipewire_query(rate));
  setup.max_buffer_size = atoi(pipewire_query(buffer));
#else
  setup.sample_rate = 48000;
  setup.max_buffer_size = 1024;
#endif

  mgr = manager_create(&setup, &status);
  
  if (mgr == NULL)
  {
    printf("Couldn't start the jfofs manager, status: %d!\n", status);
    exit_handler(status);
  }

  if (setup.verbose > 0)
  {
    printf("jfofs manager started!\n");
    printf("mgr: %p\n", mgr );
    printf("shmem: %p\n", mgr->shmem );
  }

  sleep(-1);
    
  exit_handler(0);
  return 0;
}