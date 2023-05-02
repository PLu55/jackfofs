#ifndef __JFOFS_DSP_H__
#define __JFOFS_DSP_H__

#include <jack/jack.h>
#include <fofs.h>

#include "jfofs.h"
#include "jfofs_types.h"
#include "jfofs_private.h"

typedef struct dsp_client_s dsp_client;

struct dsp_client_s
{
  int n_fofs;
  FofBank* fof_bank;
  int n_chans;
  jack_client_t* j_client;
  jack_port_t* in_port;
  jack_port_t* out_port[MAX_CHANNELS];
};

dsp_client* dsp_client_new(setup* _setup, int* status);
void dsp_client_add(dsp_client* dsp, fof* fof);
int dsp_client_activate(dsp_client* dsp);
int dsp_client_deactivate(dsp_client* dsp);

#endif
