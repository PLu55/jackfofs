#ifndef __sin_gen_h__
#define __sin_gen_h__

#include "jfofs_private.h"
#include "jfofs_types.h"

typedef struct sin_gen_s sin_gen;

struct sin_gen_s
{
  double ang_velocity;
  double amplitude;
  jack_client_t* j_client;
  jack_port_t* out_port;
};

sin_gen* sin_gen_new(double freq, double amplitude, int* status);
void sin_gen_free(sin_gen* sgen);
int sin_gen_activate(sin_gen* sgen);
int sin_gen_deactivate(sin_gen* sgen);

#endif /* __sin_gen_h__ */
