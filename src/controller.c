#include <stdlib.h>
#include <jack/jack.h>

#include <fofs.h>

#include "controller.h"
#include "dsp.h"
#include "fofs_jack_client.h"

// jack_set_buffer_size_callback()
// jack_set_sample_rate_callback()

int controller_process(jack_nframes_t nframes, void *arg)
{
  controller* controller = (controller*) arg;
  fof_queue* q = controller->q;
  int slot_idx;
  chunk * chunk;
  chunk chunk0;
  int i = 0;
  jfofs_dsp dsp;
  
  /* TODO: fix atomic access */
  q->next_frame = q->next_frame + nframes;
  slot_idx = (q->next_frame - nframes) & q->slot_size;
  chunk0 = chunk = q->slot[slot_idx];

  /* TODO: check the implementation of fof */
  while (chunk)
  {
    fof = chunk->fof;
    while (fof)
    {
      /* round robin */
      dsp = controller->dsp[i];
      jfofs_dsp_add(dsp, fof);
      i = i++ & nclients - 1;

      fof = fof->next;
    }
    chunk = chunk-> next;
  }
  
  return 0;      
}

int controller_connect(jfofs_controller* ctrl)
{
  int status = 0;
  char* c_port = c jack_port_name(ctrl->port);
  for (int i = 0; i < ctrl->nclients; i++)
  {
    status = jack_port_connect(ctrl->jclient, jack_port_name(ctrl->port),
				jack_port_name(ctrl->dsp[i]->inport));
    if (status)
      return status;
    
  }
  for (int i = 0; i < ctrl->nclients; i++)
  {
    dsp_client* dsp = ctrl->dsp[i];
    for (int j = 0; j < ctrl->nchans; j++)
    {
      *status = jack_port_connect(dsp->jclient,
				  jack_port_name(dsp->outport[j]),
				  jack_port_name(mix->inport[j]));
      if (status)
	return status;
    }
  }
  return status;
}

jfofs_controller* jfofs_controller_new(FofsMode mode, int nclients,
				       int nchans, int n_fofs_per_client,
				       int* status)
{

  // nint nchans = fof_ModeToChannels(mode);
  
  const char *client_name = "jfofs_controller";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jfofs_controller* ctrl;
  fof_queue* q;
  jack_nframes_t sample_rate;
  jack_nframes_t buf_size;
  
  *status = posix_memalign((void**) &ctrl, CACHE_LINE_SIZE,
			   sizeof(jfofs_controller));
  if (ctrl = NULL)
  {
    return NULL;
  }
  ctrl->active = 0;
  ctrl->nclients = nclients;
  ctrl->nchans = nchans;
  ctrl->n_fofs_per_client = n_fofs_per_client;
  ctrl->mode = mode;

  ctrl->j_client = jack_client_open(client_name, options, &status,
					  server_name);
  if (ctrl->j_client == NULL)
  {
    return NULL;
  }
  
  sample_rate = jack_get_sample_rate(ctrl->j_client);
  buf_size = jack_get_buffer_size(ctrl->j_client);
  /* TODO: alloc_fofs??? */
  ctrl->fof_bank = fof_newBank(sample_rate, mode, alloc_fofs, buf_size);

  ctrl->q = fof_queue_new(n_slots, slot_size, n_free_chunks, _size, status);

  jack_set_process_callback (client, controller_process, (void*) ctrl);
	
  ctrl->port = jack_port_register(ctrl->j_client, "output",
					JACK_DEFAULT_AUDIO_TYPE,
					JackPortIsOutput, 0);

  for (int i = 0; i < ctrl->nclients; i++)
  {
    jack_activate(ctrl->dsp[i]);
  }
  jack_activate(ctrl->mix);
  jack_activate(ctrl);
  controller_connect(ctrl);
  /* TODO: protect */
  ctrl->active = 1;
  return ctrl;
}

void jfofs_controller_free(jfofs_controller* ctrl)
{
  /* TODO: protect */
  ctrl->active = 0;
  for (int i = 0; i < ctrl->nclients; i++)
  {
    jack_deactivate(ctrl->dsp[i]);
  }
  jack_deactivate(ctrl->mix);
  jack_deactivate(ctrl);
}
