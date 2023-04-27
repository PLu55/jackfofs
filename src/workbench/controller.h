#ifndef __JFOFS_CONTROLLER_H__
#define __JFOFS_CONTROLLER_H__

#include <fofs.h>

#include "jfofs_types.h"

#define MAX_DSP_CLIENTS 8

struct jfofs_controller_s
{
  int active;
  int nclients;
  int nchans;
  FofsMode mode;
  jack_client_t* j_client;
  dsp_client* dsp[MAX_DSP_CLIENTS];
  mix_client* mix;
  jack_port_t* port;
  fof_queue* q;
  FofBank* fof_bank;
};

jfofs_controller* jfofs_controller_new(FofsMode mode, int nclients, int nchans,
				       int *status);
void jfofs_controller_free(jfofs_controller* mixer);

#endif /* __jfofs_controller_h__ */
