#ifndef __SETUP_H__
#define __SETUP_H__

#include "jfofs_types.h"

typedef struct setup_s setup_t;

struct setup_s
{
  int mode;              /* type of fof, defines the number of channels */
  int fofs_trace_level;      /* sets the trace level of the fofs lib */
  int n_clients;             /* number of parallel dsp clients */  
  int n_preallocate_fofs;    /* number of pre allocated fofs in each client */
  int n_max_fofs;            /* maximum number of fofs */
  int n_slots;               /* number of slots in circular fof buffer */
  int sample_rate;           /* sample (frames) per second,
			      * filled by manager to what jack has */
  int buffer_size;           /* number of samples (frames) in each cycle,
			      * set by manager to what jack has */
  int xrun_limit;            /* terminates the server if more xruns then the limit */
};

#endif
