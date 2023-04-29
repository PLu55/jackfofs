#ifndef __JFOFS_MIX_H__
#define __JFOFS_MIX_H__

#include <jack/jack.h>

#include "jfofs.h"
#include "jfofs_types.h"

struct mix_client_s
{
  jfofs_controller *ctrl;
  jack_client_t* j_client;
  jack_port_t* in_port[JFOFS_MAX_CHANS];
  jack_port_t* out_port[JFOFS_MAX_CHANS];
}

#endif
