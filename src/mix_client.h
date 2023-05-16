#ifndef __JFOFS_MIX_H__
#define __JFOFS_MIX_H__

#include <jack/jack.h>

#include "jfofs.h"
#include "jfofs_types.h"
#include "jfofs_private.h"

struct mix_client_s
{
  jack_client_t* j_client;
  jack_port_t* in_port[MAX_CHANNELS];
  jack_port_t* out_port[MAX_CHANNELS];
  fof_queue_t* q;
  int n_chans;
};

mix_client_t* mix_client_new(int n_chans, fof_queue_t* q, int* status);
int mix_client_activate(mix_client_t* mix);
int mix_client_deactivate(mix_client_t* mix);
void mix_client_free(mix_client_t* mix);

#endif
