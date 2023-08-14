#ifndef __JFOFS_H__
#define __JFOFS_H__

#include <stdint.h>
#include "jfofs_types.h"
//#include "api.h"

typedef struct jfofs_s jfofs_t;
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

jfofs_t* jfofs_new(int* status);
void jfofs_free(jfofs_t* jfofs);
int jfofs_add(jfofs_t* jfofs, jfofs_time_t time_us, float* fof_argv);
jfofs_time_t jfofs_get_time(jfofs_t* jfofs);
int jfofs_sample_rate(jfofs_t* jfofs);
void* jfofs_get_statistics(jfofs_t* jfofs);
setup_t* jfofs_get_setup(jfofs_t* jfofs);
int jfofs_get_reference_cnt(jfofs_t* jfofs);

#endif
