#ifndef __sin_gen_h__
#define __sin_gen_h__

#include "jfofs_types.h"

struct sin_gen_s
{
  double ang_velocity;
  double amplitude;
  int nchans;
  jack_client_t* j_client;
  jack_port_t* port[JFOFS_MAX_CHANS];
};

sin_gen* sin_gen_new(double freq, double amplitude, int nchans,
		     jfofs_status* status);

void sin_gen_free(sin_gen* sgen);
int sin_gen_process(jack_nframes_t nframes, void *arg);

#endif /* __sin_gen_h__ */
