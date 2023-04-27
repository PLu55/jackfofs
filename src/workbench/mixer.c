#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

jfofs_mixer*  jfofs_mixer_new(int sample_rate, int nclients, int nchans,
			      jfofs_status *status)
{
  char str[80];
  char nstr[16];
  const char *client_name = "mixer";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  jfofs_mixer* mixer = malloc (sizeof(jfofs_mixer));

  if (mixer = NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }

  mixer->j_client = jack_client_open (client_name, options, &jstatus, server_name);
  
  if (mixer->j_client == NULL)
  {
    *status = JFOFS_JACK_ERROR | jstatus;
    return NULL;
  }
  
  mixer->nclients = nclients;
  mixer->nchans = nchans;

  for (int i; i < nclients; ++i)
    {
    for (int j; j < nchans; ++j)
    {
      int k = i * nchans + j;
      sprintf(nstr, "%d", i+1);
      strcpy(str, "input_");
      strcat(str, nstr);
      strcat(str, "_");
      sprintf(nstr, "%d", j+1);
      strcat(str, nstr);
      mixer->in_port[k] = jack_port_register (mixer->j_client, str,
				    JACK_DEFAULT_AUDIO_TYPE,
				    JackPortIsInput, 0);
      if (mixer->in_port[k] == NULL)
      {
	*status = JFOFS_PORT_ERROR;
	return NULL;
      } 
    }
  }
  
  for (int i; i < nchans; ++i)
  {
    char str[80];
    char nstr[2];
    itoa(i, nstr, 10);
    strcpy(str, "output_");
    strcat(str, nstr);
    mixer->out_port[i] = jack_port_register (mixer->j_client, str,
					     JACK_DEFAULT_AUDIO_TYPE,
					     JackPortIsOutput, 0);
    if (mixer->out_port[i] == NULL)
    {
      *status = JFOFS_PORT_ERROR;
      return NULL;
    }
  }

  jack_set_process_callback (mixer->j_client, mixer_process, mixer);
  jack_activate(mixer->j_client);
  
  return client;  
}

void jfofs_mixer_free(jfofs_mixer* mixer)
{
  jack_deactivate(mixer->j_client);

  for (int i = 0; i < mixer->nclients * mixer->nchans; ++i)
  {
     jack_port_unregister(mixer->j_client, mixer->in_port[i]);
  }
  for (int i = 0; i < mixer->nchans; ++i)
  {
     jack_port_unregister(mixer->j_client, mixer->out_port[i]);
  }
  jack_client_close(mixer->j_client);
  free(mixer);
}

int mixer_process(jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t *in, *out;
  mixer* = (jfofs_mixer*) arg;

  for (int j = 0; j < mixer->nchans; ++j)
  {
    out = jack_port_get_buffer(mixer->out_port[j], nframes);
    in = jack_port_get_buffer(mixer->in_port[j], nframes);
    memcpy(out, in, sizeof (jack_default_audio_sample_t) * nframes);
    
    for (int i = 1; i < mixer->nclients; ++i)
    {
      int k = i * nchans + j;
      in = jack_port_get_buffer(mixer->in_port[k], nframes);

      for (int n = 0; n < nframes; ++n)
      {
	*(out++) += *(in++);
      }
    }
  }
  return 0;
}
