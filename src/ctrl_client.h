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
  fof_queue_t* q;
  int n;
  int m;
  void* pad[2];
  dsp_client_t* dsp[MAX_DSP_CLIENTS];
};

ctrl_client_t* ctrl_client_new(setup_t* _setup, int* status);
int ctrl_client_activate(ctrl_client_t* ctrl);
int ctrl_client_deactivate(ctrl_client_t* ctrl);
void ctrl_client_free(ctrl_client_t* ctrl);

#endif
