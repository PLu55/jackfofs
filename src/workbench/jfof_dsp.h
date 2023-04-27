#ifndef __JFOFS_DSP_H__
#define __JFOFS_DSP_H__

#import <fof.h>

typedef struct jfofs_dsp_data_s jfofs_dsp_data;

struct jfofs_dsp_data_s
{
  jfofs_controller *controller;
  int n_fofs;
  FofBank* fof_bank;
  int n_ports;
  jack_port_t** port;
};

#endif
