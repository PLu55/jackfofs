#include <stdlib.h>
#include <jack/jack.h>

#include <fofs.h>

#include "fof_queue.h"
#include "ctrl_client.h"
#include "dsp_client.h"

#include "test_util.h"
#include <stdio.h>

int ctrl_client_process(jack_nframes_t nframes, void *arg);

ctrl_client_t* ctrl_client_new(setup_t* setup, fof_queue_t* q, int* status)
{
  ctrl_client_t* ctrl;
  const char *client_name = "jfofs_controller";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  
  *status = posix_memalign((void**) &ctrl, CACHE_LINE_SIZE,
			   sizeof(ctrl_client_t));
  if (ctrl == NULL)
  {
    return NULL;
  }
  
  ctrl->active = 0;
  ctrl->q = q;
  ctrl->n_clients = setup->n_clients;
  ctrl->mode = setup->mode;
  for (int i = 0; i < MAX_DSP_CLIENTS; i++)
    ctrl->dsp[i] = NULL;
  ctrl->last_dsp = 0;
  ctrl->n = 0;
  ctrl->m = 0;
  ctrl->syncronized = 0;
  ctrl->j_client = jack_client_open(client_name, options, &jstatus,
				    server_name);
  if (ctrl->j_client == NULL)
  {
    return NULL;
  }
  
  jack_set_process_callback (ctrl->j_client, ctrl_client_process,
			     (void*) ctrl);
  ctrl->port = jack_port_register(ctrl->j_client, "out",
				  JACK_DEFAULT_AUDIO_TYPE,
				  JackPortIsOutput, 0);
  return ctrl;
}

int ctrl_client_activate(ctrl_client_t* ctrl)
{
  ctrl->active = 1;
  return jack_activate(ctrl->j_client);
}

int ctrl_client_deactivate(ctrl_client_t* ctrl)
{
  ctrl->active = 0;
  return jack_deactivate(ctrl->j_client);
}

void ctrl_client_free(ctrl_client_t* ctrl)
{
  jack_deactivate(ctrl->j_client);
  jack_client_close(ctrl->j_client);
  free(ctrl);
}

int ctrl_client_process(jack_nframes_t nframes, void *arg)
{
  ctrl_client_t* ctrl = (ctrl_client_t*) arg;
  fof_queue_t* q = ctrl->q;
  unsigned int slot_idx;
  fof_t* fof;
  int i = 0;
  dsp_client_t* dsp;
  uint64_t n;

  if (ctrl->active == 0)
    return 0;
  
  /* TODO: fix atomic access, what memorder to use? */
  n = __atomic_add_fetch(&q->next_frame, nframes, __ATOMIC_ACQ_REL);
  n -= nframes;
#ifdef TRACE
  printf("ctrl_client n: %ld\n", n);
  for (int i = 0; i < ctrl->n_clients; i++)
    printf("   dsp[%d] n: %ld\n", i, dsp_get_next_frame(ctrl->dsp[i]));
#endif
  if (!ctrl->syncronized)
  {
    for (int i = 0; i < ctrl->n_clients; i++)
      dsp_set_next_frame(ctrl->dsp[i], n);
    ctrl->syncronized = 1;
  }
    
  slot_idx = (n / q->buffer_size) & (q->n_slots - 1);
  fof =  q->slot[slot_idx];
  /* TODO: check how this must be handled! */

  __atomic_store_n(q->slot[slot_idx], NULL, __ATOMIC_RELEASE);
  q->slot[slot_idx] = NULL;
#ifdef TRACE
  if (fof)
  {
    printf("Has fof: %p @ n: %ld\n", fof, n);
    fof_print(fof);
  }
#endif
  /* TODO: check the implementation of fof */
  i = ctrl->last_dsp;
  q->first_fof = fof;
  while (fof)
  {
    /* round robin, distribute the work over available dsp_clients */
    dsp = ctrl->dsp[i++];
    i %= ctrl->n_clients;
    dsp_client_add(dsp, fof);
    q->last_fof = fof;
    fof = fof->next;
  }
  ctrl->last_dsp = i;
  /* the fofs are added to the free list by the mix_client, this
   * allows the dsp_clients to compute the fof data for the fofs 
   * library.
   */ 
  return 0;      
}


