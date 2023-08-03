#include <stdlib.h>
#include <stdio.h> /* debug */
#include <jack/jack.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "ctrl_client.h"
#include "dsp_client.h"

int dsp_client_process (jack_nframes_t nframes, void *arg);

dsp_client_t* dsp_client_new(setup_t* setup, int idx, int* status)
{
  dsp_client_t* dsp;
  const char *client_name_base = "jfofs_dsp";
  char client_name[64];
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  jack_nframes_t sample_rate;
  jack_nframes_t buffer_size;

  sprintf( client_name, "%s%02d", client_name_base, idx + 1);

  *status = posix_memalign((void**) &dsp, CACHE_LINE_SIZE, sizeof(dsp_client_t));
  if (dsp == NULL)
  {
    return NULL;
  }

  dsp->n_fofs = 0;
  dsp->n_chans = fof_ModeToChannels(setup->mode);
  dsp->j_client = jack_client_open(client_name, options, &jstatus, server_name);
  
  if (dsp->j_client == NULL)
  {
    free(dsp);
    return NULL;
  }

  jack_set_process_callback(dsp->j_client , dsp_client_process, (void*) dsp);

  sample_rate = jack_get_sample_rate(dsp->j_client);
  buffer_size = jack_get_buffer_size(dsp->j_client);
    
  dsp->fof_bank = fof_newBank(sample_rate, setup->mode,
			      setup->n_preallocate_fofs, (int) buffer_size);
  if (dsp->fof_bank == NULL)
  {
    free(dsp);
    return NULL;
  }
  fof_set_trace_level(setup->fofs_trace_level);
    
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

uint64_t dsp_get_next_frame(dsp_client_t* dsp)
{
  return fof_timeI(dsp->fof_bank);
}

void dsp_set_next_frame(dsp_client_t* dsp, uint64_t n)
{
  fof_set_timeI(dsp->fof_bank, n);
}

int dsp_client_activate(dsp_client_t* dsp)
{
  return jack_activate(dsp->j_client);
}

int dsp_client_deactivate(dsp_client_t* dsp)
{
  return jack_deactivate(dsp->j_client);
}

/*
 * memory layout of buffers:
 *--------------------------------- 
 * ptr-1
 *---------------------------------
 * ptr-2
 *---------------------------------
 *    ...
 *---------------------------------
 * ptr-n
 *---------------------------------
 * float arr-1[]
 *---------------------------------
 * float arr-2[]
 *---------------------------------
 *   ...
 *---------------------------------
 * float arr-1[]
 *---------------------------------
 *
 */

int dsp_client_process (jack_nframes_t nframes, void *arg)
{
  dsp_client_t* dsp = (dsp_client_t*) arg;
  jack_default_audio_sample_t *buf[MAX_CHANNELS];

  for(int i = 0; i < dsp->n_chans; i++)
  {
    buf[i] = jack_port_get_buffer(dsp->out_port[i], nframes);
  }
  
  fof_next(dsp->fof_bank, nframes, buf);

  return 0;
}

void dsp_client_free(dsp_client_t* dsp)
{
  dsp_client_deactivate(dsp);
  jack_client_close(dsp->j_client);
  fof_freeBank(dsp->fof_bank);
  free(dsp);
}
