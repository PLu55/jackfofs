#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include "sin_gen.h"
#include "jfofs_types.h"

#define PI2 (M_PI * 2.0)

int sin_gen_process(jack_nframes_t nframes, void *arg);

sin_gen_t *sin_gen_new(double freq, double amplitude, int *status)
{
  sin_gen_t *sgen = NULL;
  const char *client_name = "sin_gen";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;

  *status = posix_memalign((void **)&sgen, CACHE_LINE_SIZE, sizeof(sin_gen_t));

  if (*status != 0 || sgen == NULL)
  {
    return NULL;
  }

  sgen->j_client = jack_client_open(client_name, options, &jstatus, server_name);

  if (sgen->j_client == NULL)
  {
    *status = JFOFS_JACK_ERROR_MASK | jstatus;
    return NULL;
  }

  sgen->ang_velocity = PI2 * freq / (double)jack_get_sample_rate(sgen->j_client);
  sgen->amplitude = amplitude;

  sgen->out_port = jack_port_register(sgen->j_client, "output",
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);
  if (sgen->out_port == NULL)
  {
    *status = JFOFS_PORT_ERROR;
    return NULL;
  }
  jack_set_process_callback(sgen->j_client, sin_gen_process, sgen);

  return sgen;
}

int sin_gen_activate(sin_gen_t *sgen)
{
  return jack_activate(sgen->j_client);
}

int sin_gen_deactivate(sin_gen_t *sgen)
{
  return jack_deactivate(sgen->j_client);
}

void sin_gen_free(sin_gen_t *sgen)
{
  jack_deactivate(sgen->j_client);
  jack_client_close(sgen->j_client);
  free(sgen);
}

int sin_gen_process(jack_nframes_t nframes, void *arg)
{
  sin_gen_t *sgen = (sin_gen_t *)arg;
  jack_default_audio_sample_t *buf;
  jack_nframes_t nframes0 = jack_last_frame_time(sgen->j_client);

  buf = jack_port_get_buffer(sgen->out_port, nframes);
  for (jack_nframes_t i = 0; i < nframes; ++i)
  {
    *(buf++) = sgen->amplitude * sin(sgen->ang_velocity * (double)(i + nframes0));
  }

  return 0;
}
