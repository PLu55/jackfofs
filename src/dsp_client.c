#include <stdlib.h>
#include <stdio.h> /* debug */

#include "jfofs_private.h"
#include "ctrl_client.h"
#include "dsp_client.h"

int dsp_client_process (jack_nframes_t nframes, void *arg);

dsp_client* dsp_client_new(setup* _setup, int* status)
{
  dsp_client* dsp;
  const char *client_name = "jfofs_dsp";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  jack_nframes_t sample_rate;
  jack_nframes_t buffer_size;

  *status = posix_memalign((void**) &dsp, CACHE_LINE_SIZE, sizeof(dsp_client));
  if (dsp == NULL)
  {
    return NULL;
  }

  dsp->n_fofs = 0;
  dsp->n_chans = fof_ModeToChannels(_setup->mode);
  dsp->j_client = jack_client_open(client_name, options, &jstatus, server_name);
  
  if (dsp->j_client == NULL)
  {
    free(dsp);
    return NULL;
  }

  jack_set_process_callback(dsp->j_client , dsp_client_process, (void*) dsp);

  sample_rate = jack_get_sample_rate(dsp->j_client);
  buffer_size = jack_get_buffer_size(dsp->j_client);
    
  dsp->fof_bank = fof_newBank(sample_rate, _setup->mode,
			      _setup->n_preallocate_fofs, (int) buffer_size);
  if (dsp->fof_bank == NULL)
  {
    free(dsp);
    return NULL;
  } 

  dsp->in_port = jack_port_register(dsp->j_client, "in",
				    JACK_DEFAULT_AUDIO_TYPE,
				    JackPortIsInput, 0);
  
  if (dsp->in_port == NULL)
  {
    return NULL;
  }
  for (int i = 0; i < dsp->n_chans; i++)
  {
    char name[64];
    
    sprintf (name, "out_%d", i+1);
    dsp->out_port[i] = jack_port_register(dsp->j_client, name,
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
    if (dsp->out_port[i] == NULL)
    {
      return NULL;
    }
  }

  return dsp;
}

void dsp_client_activate(dsp_client* dsp)
{
  jack_activate(dsp->j_client);
}

void dsp_client_deactivate(dsp_client* dsp)
{
  jack_deactivate(dsp->j_client);
}

void dsp_client_add(dsp_client* dsp, fof* fof)
{
  fof_add_v(dsp->fof_bank, fof->time_us, fof->argv);
  
  /* TODO: how to count fofs? 
   * Implement a counter in fofs.c
   */
}

int dsp_client_process (jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t *buf[JFOFS_MAX_CHANS];
  dsp_client* dsp = (dsp_client*) arg;

  for(int i = 0; i < dsp->n_chans; i++)
  {
    buf[i] = jack_port_get_buffer(dsp->out_port[i], nframes);
  }
  
  fof_next(dsp->fof_bank, nframes, buf);

  return 0;
}
