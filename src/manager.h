#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "jfofs_private.h"
#include "ctrl_client.h"
#include "dsp_client.h"
#include "mix_client.h"

typedef struct manager_s manager;

struct manager_s
{
  ctrl_client* ctrl;
  dsp_client* dsp[MAX_DSP_CLIENTS];
  mix_client* mix;
  setup setup;
};

manager* manager_new(int *status);
int manager_activate_clients(manager* mgr);
int manager_connect_clients(manager* mgr);

#endif
