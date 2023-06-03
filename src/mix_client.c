#include <jack/jack.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "mix_client.h"
#include "fof_queue.h"
#include "debug.h"

int mix_client_process (jack_nframes_t nframes, void *arg);

mix_client_t* mix_client_new(int n_chans, fof_queue_t* q, int* status)
{
  mix_client_t* mix;
  const char *client_name = "jfofs_mix";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  
  *status = posix_memalign((void**) &mix, CACHE_LINE_SIZE, sizeof(mix_client_t));
  if (mix == NULL)
    return NULL;
  
  mix->q = q;
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
  mix_client_t* mix = (mix_client_t*) arg;
  jack_default_audio_sample_t *in_buf;
  jack_default_audio_sample_t *out_buf;
  fof_queue_t* q = mix->q;

  ADD_CTRL_ENTRY_CNT(-1);
  
  //printf("n_chans: %d\n", mix->n_chans);
  for(int i = 0; i < mix->n_chans; i++)
  {
    in_buf = jack_port_get_buffer(mix->in_port[i], nframes);
    out_buf = jack_port_get_buffer(mix->out_port[i], nframes);
    //printf("i: %d in_buf: %p out_buf:%p\n", i, in_buf, out_buf);

    memcpy(out_buf, in_buf, nframes * sizeof(jack_default_audio_sample_t));
    //memset(out_buf, 0, nframes * sizeof(jack_default_audio_sample_t));
  }
#if 0
  if (q->first_fof != NULL)
  {
    //printf("mix_client_process release fofs\n");
    fof_queue_free_fofs(q, q->first_fof, q->last_fof);
    q->first_fof = NULL;
    q->last_fof = NULL;
  }
#endif
  //printf("mix_client_process <-----\n");
  return 0;
}

int mix_client_activate(mix_client_t* mix)
{
  return jack_activate(mix->j_client);
}

int mix_client_deactivate(mix_client_t* mix)
{
  return jack_deactivate(mix->j_client);
}

void mix_client_free(mix_client_t* mix)
{
  jack_deactivate(mix->j_client);
  jack_client_close(mix->j_client);
  free(mix);
}
