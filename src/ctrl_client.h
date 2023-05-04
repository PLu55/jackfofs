#ifndef __JFOFS_CONTROLLER_H__
#define __JFOFS_CONTROLLER_H__

#include <fofs.h>
#include <jack/jack.h>

#include "jfofs_types.h"
#include "jfofs_private.h"

struct ctrl_client_s
{
  int active;
  int n_clients;
  FofMode mode;
  jack_client_t* j_client;
  jack_port_t* port;
  fof_queue* q;
  int n;
  int m;
  void* pad[2];
  dsp_client* dsp[MAX_DSP_CLIENTS];
};

ctrl_client* ctrl_client_new(setup* _setup, int* status);
int ctrl_client_activate(ctrl_client* ctrl);
int ctrl_client_deactivate(ctrl_client* ctrl);
void ctrl_client_free(ctrl_client* ctrl);

#endif
