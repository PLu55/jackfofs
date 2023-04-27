#ifndef __jfofs_mixer_h__
#define __jfofs_mixer_h__

#include "jfofs_types.h"

typedef struct jfofs_mixer_s jfofs_mixer;

jfofs_mixer_s
{
  int nclients;
  int nchans;
  jack_client_t* j_client;
  jack_port_t* in_port[JFOFS_MAX_CHANS];
  jack_port_t* out_port[JFOFS_MAX_CHANS];
};

jfofs_mixer* jfofs_mixer_new(int sample_rate, int nclients, int nchans,
			     jfofs_status *status);
void jfofs_mixer_free(jfofs_mixer* mixer);

#endif /* __jfofs_mixer_h__ */
