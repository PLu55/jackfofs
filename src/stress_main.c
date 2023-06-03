#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <argp.h>
#include <fofs.h>

#include "jfofs.h"
#include "test_util.h"
#include "config.h"

static char doc[] =
  "jfofsstress is a program that put pressure on the jfofs server";

static struct argp_option options[] = {
  {"verbose",     'v', 0,            0, "Produce verbose output" },
  {"adur",        'a', "f",          0, "duration (alpha) of a single fof in s, "}, 
  {"bdur",        'b', "f",          0, "duration of the attack (beta) of a single fof in s, "}, 
  {"sleep",       's', "t",          0, "time between fofs in µs"}, 
  {"latancy",     'l', "l",          0, "how far ahead the fofs are added in µs"}, 
  { 0 }
};

struct arguments
{
  int verbose;
  double adur;
  double bdur;
  int sleep;
  int latancy;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'a':
      arguments->adur = atof(arg);
      break;
    case 'b':
      arguments->bdur = atof(arg);
      break;
    case 's':
      arguments->sleep = atoi(arg);
      break;
    case 'l':
      arguments->latancy = atoi(arg);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };

static jfofs_t* jfofs = NULL;

static inline void sleep_us(jfofs_time_t t)
{
  struct timespec ts;
  struct timespec tr;
  
  ts.tv_sec = t / 1000000UL;
  ts.tv_nsec = (t - ts.tv_sec) * 1000UL;
  nanosleep(&ts, &tr);
}

static void exit_handler(void)
{
  if (jfofs != NULL)
  {
    jfofs_free(jfofs);
  }
}

static void termination_handler (int signum)
{
  printf("terminating: %d\n", signum);
  exit(-1);
}

static void setup_signal_handlers(void)
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

int main(int argc, char** argv)
{
  int status;
  fof_t fof;
  jfofs_time_t t;
  uint64_t cnt = 0;
  int tcnt = 0;
  int s6_cnt = 0;
  
  struct arguments arguments;
  
  arguments.verbose = 0;
  arguments.adur = 0.01;
  arguments.bdur = 0.001;
  arguments.sleep = 2000;
  arguments.latancy = 500; 

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  atexit(exit_handler);
  setup_signal_handlers();
  jfofs =  jfofs_new(&status);

  if (jfofs == NULL)
  {
    printf("Can't create a jfofs client!");
    exit(-1);
  }
  
  fof_default(&fof);
  fof.argv[FOF_ARG_beta] = arguments.bdur;
  fof.argv[FOF_ARG_alpha] = fof_t60_to_alpha(arguments.adur);

  for (;;)
  {
    t = jfofs_get_time(jfofs) + arguments.latancy;
    status =  jfofs_add(jfofs, t, fof.argv);
    if (status != 0)
    {
      printf("status: %d\n", status);
      if (status == 6 && ++s6_cnt > 10)
      {
	printf("To many status 6, exiting now.");
	exit(-1);
      }
    }
    sleep_us(arguments.sleep);
    if (++cnt > 200UL)
    {
      cnt = 0;
      printf(".");
      fflush(stdout);
      if (++tcnt > 60)
      {
	printf("\n");
	tcnt = 0;
      }
    }
  }
}
