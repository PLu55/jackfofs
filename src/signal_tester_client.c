#include <jack/jack.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "signal_tester_client.h"
#include "jfofs_private.h"

int stc_process (jack_nframes_t nframes, void *arg);

signal_tester_client* signal_tester_client_new(int* status)
{
  signal_tester_client* stc;
  const char *client_name = "signal_tester";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
    
  *status = posix_memalign((void**) &stc, CACHE_LINE_SIZE,
			   sizeof(signal_tester_client));
  if (stc == NULL)
  {
    return NULL;
  }
  signal_tester_client_reset(stc);

  stc->j_client = jack_client_open(client_name, options, &jstatus,
				   server_name);

  if (stc->j_client == NULL)
  {
    free(stc);
    return NULL;
  }

  jack_set_process_callback(stc->j_client, stc_process, (void*) stc);

  stc->in_port = jack_port_register(stc->j_client, "in",
				    JACK_DEFAULT_AUDIO_TYPE,
				    JackPortIsInput, 0);
  if (stc->in_port == NULL)
  {
    free(stc);
    return NULL;
  }

  return stc;
}

void signal_tester_client_reset(signal_tester_client* stc)
{
  stc->n = 0;
  stc->sum = 0.0f;
  stc->min = FLT_MAX;
  stc->max = FLT_MIN;
}

void signal_tester_client_free(signal_tester_client* stc)
{
  if (stc->j_client != NULL)
  {
    jack_deactivate(stc->j_client);
    jack_client_close(stc->j_client);
  }
  free(stc);
}

float signal_tester_client_rms(signal_tester_client* stc)
{
  return sqrtf(stc->sum / stc->n);
}

void signal_tester_client_activate(signal_tester_client* stc)
{
  jack_activate(stc->j_client);
}

void signal_tester_client_deactivate(signal_tester_client* stc)
{
  jack_deactivate(stc->j_client);
}

int stc_process (jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t *buf;
  signal_tester_client* stc = (signal_tester_client*) arg;

  buf = jack_port_get_buffer(stc->in_port, nframes);

  for (int i = 0; i < nframes && stc->n < stc->m; i++)
  {
    float x = buf[i];
    stc->sum += x * x;
    stc->min = x < stc->min ? x : stc->min; 
    stc->max = x > stc->max ? x : stc->max;
    stc->n++;
  }

  return 0;
}

