#ifndef __jfofs_controller_h__
#define __jfofs_controller_h__

#include "jfofs_types.h"

jfofs_controller_s
{
  int nclients;
  int nchans;
  jack_client_t* j_client;
  jack_port_t* port;
};

jfofs_mixer* jfofs_controller_new(int sample_rate, int nclients, int nchans,
			     jfofs_status *status);
void jfofs_controller_free(jfofs_mixer* mixer);

#endif /* __jfofs_controller_h__ */
