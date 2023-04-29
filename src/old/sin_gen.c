#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include "sin_gen.h"
#include "jfofs_types.h"

#define PI2 (M_PI * 2.0)

int sin_gen_process(jack_nframes_t nframes, void *arg)
{
  sin_gen* sgen = (sin_gen*) arg;
  jack_default_audio_sample_t *buf, *buf0;
  jack_nframes_t nframes0 = jack_last_frame_time(sgen->j_client);

  
  buf = buf0 = jack_port_get_buffer(sgen->port[0], nframes);
  for (int i = 0; i < nframes; ++i)
  {
    *(buf++) = sgen->amplitude * sin(sgen->ang_velocity * (double)(i + nframes0));
  }

  for (int i = 1; i < sgen->nchans; ++i)
  {
    buf = jack_port_get_buffer (sgen->port[i], nframes);
    memcpy (buf, buf0, sizeof (jack_default_audio_sample_t) * nframes);
  }
  return 0;      
}

sin_gen* sin_gen_new(double freq, double amplitude, int nchans, jfofs_status* status)
{
  char str[80];
  char nstr[16];
  const char *client_name = "sin_gen";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;

  sin_gen* sgen = (sin_gen*) calloc(1, sizeof(sin_gen));
  if (sgen == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  sgen->nchans = nchans;
  sgen->j_client = jack_client_open (client_name, options, &jstatus, server_name);
  
  if (sgen->j_client == NULL)
  {
    *status = JFOFS_JACK_ERROR | jstatus;
    return NULL;
  }
  
  sgen->ang_velocity = PI2 * freq / (double) jack_get_sample_rate(sgen->j_client);
  sgen->amplitude = amplitude;

  for (int i = 0; i < sgen->nchans; ++i)
  {
    sprintf(nstr, "%d", i+1);
    strcpy(str, "output_");
    strcat(str, nstr);
    sgen->port[i] = jack_port_register(sgen->j_client, str,
				       JACK_DEFAULT_AUDIO_TYPE,
				       JackPortIsOutput, 0);
    if ( sgen->port[i] == NULL)
    {
	*status = JFOFS_PORT_ERROR;
	return NULL;
    }
  }
  jack_set_process_callback (sgen->j_client, sin_gen_process, sgen);
  jack_activate(sgen->j_client);
  
  return sgen;
}

void sin_gen_free(sin_gen* sgen)
{
  jack_deactivate(sgen->j_client);
  for (int i = 0; i < sgen->nchans; ++i)
  {
    jack_port_unregister(sgen->j_client, sgen->port[i]);
  }
  jack_client_close(sgen->j_client);
  free(sgen);
}
