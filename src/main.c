#include <stdlib.h>
#include <unistd.h>
#include <argp.h>
#include <signal.h>

#include "config.h"
#include "jfofs_private.h"
#include "manager.h"

const char *argp_program_version = PROJECT_NAME_VER;

static char doc[] =
  "jfofs is a jack client program that is a parallel fof synthesizer";

static struct argp_option options[] = {
  {"verbose",     'v', 0,            0, "Produce verbose output" },
  {"mode",        'm', "mode",       0,
   "Fof mode: 1: mono, 2: stereo 3: quad: 4: ambiO1 5: ambi01D, default is 1" },
  {"n_clients",   'n', "n",  0, "number of parallel clients, default is 1" },
  {"n_fofs",      'p', "n",     0, "number of preallocated fofs in libfofs, default is 10240" },
  {"n_slots",     's', "n",    0, "number of slots in circular queue, default is 32" },
  {"n_max_fofs",  'q', "n", 0, "maximum number of fofs in queue, default is 1024" },
  {"trace_level", 't', "trace",      0, "sets the trace level, default is 0" },    
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
  if (mgr != NULL)
  {
    manager_deactivate_clients(mgr);
    manager_free(mgr);
  }

  exit(status != 0);
}

static void termination_handler (int signum)
{
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

  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGHUP, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGHUP, &new_action, NULL);
  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTERM, &new_action, NULL);

  arguments.verbose = 0;
  arguments.mode = 1;
  arguments.n_clients = 1;
  arguments.n_fofs = 10 * 1024;
  arguments.n_max_fofs = 1024;
  arguments.n_slots = 32;
  arguments.trace_level = 0;
  
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  setup.mode = arguments.mode;
  setup.n_clients = arguments.n_clients;
  setup.n_preallocate_fofs =  arguments.n_fofs;
  setup.n_slots =  arguments.n_slots;
  setup.n_max_fofs = arguments.n_max_fofs;
  setup.fofs_trace_level = arguments.trace_level;
  
  mgr = manager_create(&setup, &status);
  
  if (mgr == NULL)
  {
    printf("Couldn't start the jfofs manager, status: %d!\n", status);
    exit_handler(status);
  }

  sleep(-1);
    
  exit_handler(0);
  return 0;
}
