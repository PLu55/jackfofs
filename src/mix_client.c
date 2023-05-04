#include <jack/jack.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "mix_client.h"

int mix_client_process (jack_nframes_t nframes, void *arg);

mix_client* mix_client_new(int n_chans, int* status)
{
  mix_client* mix;
  const char *client_name = "jfofs_mix";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  
  *status = posix_memalign((void**) &mix, CACHE_LINE_SIZE, sizeof(mix_client));
  if (mix == NULL)
    return NULL;

  mix->j_client = jack_client_open(client_name, options, &jstatus, server_name);
  if (mix->j_client == NULL)
  {
    free(mix);
    return NULL;
  }
  jack_set_process_callback (mix->j_client, mix_client_process, mix);
  
  mix->n_chans = n_chans;
  for (int i = 0; i < mix->n_chans; i++)
  {
    char name[64];

    sprintf (name, "in_%d", i+1);
    mix->in_port[i] = jack_port_register(mix->j_client, name,
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
    if (mix->in_port[i] == NULL)
    {
      return NULL;
    }

    sprintf (name, "out_%d", i+1);
    mix->out_port[i] = jack_port_register(mix->j_client, name,
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
    if (mix->out_port[i] == NULL)
    {
      return NULL;
    }
  }

  return mix;
}

int mix_client_process (jack_nframes_t nframes, void *arg)
{
  mix_client* mix = (mix_client*) arg;
  jack_default_audio_sample_t *in_buf;
  jack_default_audio_sample_t *out_buf;
  //printf("n_chans: %d\±n", mix->n_chans);
  for(int i = 0; i < mix->n_chans; i++)
  {
    in_buf = jack_port_get_buffer(mix->in_port[i], nframes);
    out_buf = jack_port_get_buffer(mix->out_port[i], nframes);
    //printf("i: %d in_buf: %p out_buf:%p\n", i, in_buf, out_buf);

    memcpy(out_buf, in_buf, nframes * sizeof(jack_default_audio_sample_t));
    //memset(out_buf, 0, nframes * sizeof(jack_default_audio_sample_t));
  }
    return 0;
}

int mix_client_activate(mix_client* mix)
{
  return jack_activate(mix->j_client);
}

int mix_client_deactivate(mix_client* mix)
{
  return jack_deactivate(mix->j_client);
}

void mix_client_free(mix_client* mix)
{
  jack_deactivate(mix->j_client);
  jack_client_close(mix->j_client);
  free(mix);
}
