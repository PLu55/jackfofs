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
  {"verbose",   'v', 0,            0,  "Produce verbose output" },
  {"mode",      'm', "mode",       0,  "Fof mode: 1: mono, 2: stereo 3: quad: 4: ambiO1 5: ambi01D" },
  {"n_clients", 'n', "n_clients",  0,  "number of parallel clients" },
  {"n_fofs",    'p', "n_fofs",     0,  "number of preallocated fofs in" },
  {"n_slots",   's', "n_slots",    0,  "number of slots in circular queue" },
  {"chunk_size",'c', "chunk_size", 0,  "number of fofs per chunk" },
  {"n_chunks",  'k', "n_chunks",   0,  "number of chunk to allocate" },
  { 0 }
};

struct arguments
{
  int verbose;
  int mode;
  int n_clients;
  int n_fofs;
  int n_slots;
  int chunk_size;
  int n_chunks;
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
    case 'c':
      arguments->chunk_size = atoi(arg);
      break;
    case 'k':
      arguments->n_chunks = atoi(arg);
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
  setup _setup;
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
  arguments.n_fofs = 1024;
  arguments.n_slots = 64;
  arguments.chunk_size = 128;
  arguments.n_chunks = 256;
  
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  _setup.mode = arguments.mode;
  _setup.n_clients = arguments.n_clients;
  _setup.n_preallocate_fofs =  arguments.n_fofs;
  _setup.n_slots =  arguments.n_slots;
  _setup.chunk_size = arguments.chunk_size;
  _setup.n_free_chunks = arguments.n_chunks;

  mgr = manager_create(&_setup, &status);
  
  if (mgr == NULL)
  {
    printf("Couldn't start the jfofs manager, status: %d!\n", status);
    exit_handler(status);
  }

  manager_ipc_loop(mgr);
    
  exit_handler(0);
  return 0;
}
