#ifndef __JFOFS_DSP_H__
#define __JFOFS_DSP_H__

#include <fofs.h>

#include "jfofs.h"
#include "jfofs_types.h"


typedef struct dsp_client_s dsp_client;

struct dsp_client_s
{
  jfofs_controller *controller;
  int n_fofs;
  FofBank* fof_bank;
  int n_ports;
  jack_client_t* j_client;
  jack_port_t* in_port;
  jack_port_t* out_port[JFOFS_MAX_CHANS];
};

#endif
