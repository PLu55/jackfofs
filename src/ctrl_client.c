#include <stdlib.h>
#include <jack/jack.h>

#include <fofs.h>

#include "fof_queue.h"
#include "ctrl_client.h"
#include "dsp_client.h"

// jack_set_buffer_size_callback()
// jack_set_sample_rate_callback()

int ctrl_client_process(jack_nframes_t nframes, void *arg);
int ctrl_client_connect(ctrl_client* ctrl);
void ctrl_client_activate(ctrl_client* ctrl);
void ctrl_client_deactivate(ctrl_client* ctrl);

ctrl_client* ctrl_client_new(FofMode mode, int nclients, int n_fofs_per_client,
			     int n_prealloc_fofs, int* status)
{
  ctrl_client* ctrl;
  const char *client_name = "jfofs_controller";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;

  jack_nframes_t sample_rate;
  jack_nframes_t buf_size;
  jack_status_t jstatus;

  /* queue setup */
  int n_slots = 64;
  int slot_size = 64;
  int n_free_chunks = 128;
  int chunk_size = 256;
  
  *status = posix_memalign((void**) &ctrl, CACHE_LINE_SIZE,
			   sizeof(ctrl_client));
  if (ctrl == NULL)
  {
    return NULL;
  }
  ctrl->active = 0;
  ctrl->nclients = nclients;
  ctrl->nchans = fof_ModeToChannels(mode);
  ctrl->n_fofs_per_client = n_fofs_per_client;
  ctrl->mode = mode;

  ctrl->j_client = jack_client_open(client_name, options, &jstatus,
					  server_name);
  if (ctrl->j_client == NULL)
  {
    return NULL;
  }
  
  sample_rate = jack_get_sample_rate(ctrl->j_client);
  buf_size = jack_get_buffer_size(ctrl->j_client);

  ctrl->fof_bank = fof_newBank(sample_rate, mode, n_prealloc_fofs, buf_size);

  ctrl->q = fof_queue_new(sample_rate, n_slots, slot_size, n_free_chunks,
			  chunk_size, status);

  jack_set_process_callback (ctrl->j_client, ctrl_client_process, (void*) ctrl);
	
  ctrl->port = jack_port_register(ctrl->j_client, "out",
				  JACK_DEFAULT_AUDIO_TYPE,
				  JackPortIsOutput, 0);
  jack_activate(ctrl->j_client);
#if 0
  for (int i = 0; i < ctrl->nclients; i++)
  {
    dsp_client_activate(ctrl->dsp[i]);
  }
  //jack_activate(ctrl->mix->j_client);
  jack_activate(ctrl->j_client);
  ctrl_client_connect(ctrl);
  /* TODO: protect */
  ctrl->active = 1;
#endif
  return ctrl;
}

void ctrl_client_activate(ctrl_client* ctrl)
{
  jack_activate(ctrl->j_client);
}

void ctrl_client_deactivate(ctrl_client* ctrl)
{
  jack_deactivate(ctrl->j_client);
}

void ctrl_client_free(ctrl_client* ctrl)
{
  /* TODO: protect */
  ctrl->active = 0;
  for (int i = 0; i < ctrl->nclients; i++)
  {
    dsp_client_deactivate(ctrl->dsp[i]);
  }
  //jack_deactivate(ctrl->mix);
  jack_deactivate(ctrl->j_client);
}

int ctrl_client_process(jack_nframes_t nframes, void *arg)
{
  ctrl_client* ctrl = (ctrl_client*) arg;
  fof_queue* q = ctrl->q;
  int slot_idx;
  chunk* chunk0;
  chunk* chunk_last;
  chunk* chk;
  int i = 0;
  dsp_client* dsp;
  uint64_t n;
  
  /* TODO: fix atomic access, what memorder to use? */
  n = __atomic_add_fetch(&q->next_frame, nframes, __ATOMIC_ACQ_REL);
  slot_idx = (n - nframes) & q->slot_size;
  chunk0 = chk = q->slot[slot_idx];

  q->slot[slot_idx] = 0;

  /* TODO: check the implementation of fof */
  while (chk)
  {
    chunk_last = chk;
    for (int j = 0; j < chk->count; j++)
    {
      /* round robin */
      dsp = ctrl->dsp[i];
      dsp_client_add(dsp, &(chk->fof[j]));
      i++;
      i &= (ctrl->nclients - 1);
    }
    chk = chk->next;
  }
  
  /* TODO: add chunks to free_chunks list in q */
  if (chunk0 != NULL)
  {
    chunk_last->next = q->free_chunks;
    __atomic_thread_fence(__ATOMIC_ACQ_REL);
    q->free_chunks = chunk0;
  }
  return 0;      
}

int ctrl_client_connect(ctrl_client* ctrl)
{
  int status = 0;

  for (int i = 0; i < ctrl->nclients; i++)
  {
    status = jack_connect(ctrl->j_client,
			  jack_port_name(ctrl->port),
			  jack_port_name(ctrl->dsp[i]->in_port));
    if (status)
      return status;
    
  }
  #if 0
  for (int i = 0; i < ctrl->nclients; i++)
  {
    dsp_client* dsp = ctrl->dsp[i];
    for (int j = 0; j < ctrl->nchans; j++)
    {
      *status = jack_connect(dsp->j_client,
			     jack_port_name(dsp->out_port[j]),
			     jack_port_name(mix->in_port[j]));
      if (status)
	return status;
    }
  }
  #endif
  return status;
}

