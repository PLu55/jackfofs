#ifndef __JFOFS_CONTROLLER_H__
#define __JFOFS_CONTROLLER_H__

#include <fofs.h>
#include <jack/jack.h>

#include "jfofs_types.h"
#include "jfofs_private.h"

#define MAX_DSP_CLIENTS 8


struct controller_s
{
  int active;
  int nclients;
  int nchans;
  int n_fofs_per_client;
  FofMode mode;
  jack_client_t* j_client;
  dsp_client* dsp[MAX_DSP_CLIENTS];
  mix_client* mix;
  jack_port_t* port;
  fof_queue* q;
  FofBank* fof_bank;
};

controller* controller_new(FofMode mode, int nclients, int nchans, int *status);
void controller_free(controller* ctrl);

#endif /* __jfofs_controller_h__ */
