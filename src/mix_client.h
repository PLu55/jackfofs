#ifndef __JFOFS_MIX_H__
#define __JFOFS_MIX_H__

#include <jack/jack.h>

#include "jfofs.h"
#include "jfofs_types.h"

struct mix_client_s
{
  jack_client_t* j_client;
  int n_chans;
  jack_port_t* in_port[MAX_CHANNELS];
  jack_port_t* out_port[MAX_CHANNELS];
};

mix_client* mix_client_new(int n_chans, int* status);
int mix_client_activate(mix_client* mix);
int mix_client_deactivate(mix_client* mix);
void mix_client_free(mix_client* mix);

#endif
