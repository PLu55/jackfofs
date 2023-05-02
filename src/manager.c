#include <stdlib.h>
#include <stdio.h> /* debug */
#include <jack/jack.h>

#include "manager.h"
#include "fof_queue.h"
#include "ctrl_client.h"
#include "dsp_client.h"
#include "mix_client.h"
#include "jfofs.h"
#include "jfofs_private.h"

// jack_set_buffer_size_callback()
// jack_set_sample_rate_callback()

manager* manager_new(int *status)
{
  manager* mgr;

  //jack_nframes_t n ,m;
  //fof _fof;
  //jack_nframes_t sample_rate;
  //jack_nframes_t buffer_size;

  *status = posix_memalign((void**) &mgr, CACHE_LINE_SIZE, sizeof(manager));

  if (mgr == NULL)
  {
    return NULL;
  }

  mgr->setup.mode = FOF_MONO;
  mgr->setup.n_clients = 1;
  mgr->setup.n_preallocate_fofs = 1024;
  mgr->setup.n_slots = 64;
  mgr->setup.n_free_chunks = 128;
  mgr->setup.chunk_size = 256;
  
  mgr->ctrl = ctrl_client_new(&mgr->setup, status);
  
  //sample_rate = jack_get_sample_rate(mgr->ctrl->j_client);
  //buffer_size = jack_get_buffer_size(mgr->ctrl->j_client);

  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    mgr->dsp[i] = dsp_client_new(&mgr->setup, status);
  }

  return mgr;
}

inline jfofs_time systime()
{
  struct timespec tspec;

  clock_gettime(CLOCK_REALTIME, &tspec);
  return (jfofs_time) tspec.tv_sec * TIME_1  + (jfofs_time) tspec.tv_nsec * 1000;
}

#if 0
struct PLu_Fofs : public Unit
{
  FofBank* mBank;
  int mState;
  double mDtma;
  double mTcorr;
  double mTframe;
#ifdef DEBUG
  long frameCnt;
#endif
};

/* At each buffer cycle in jack */
void time_handling()
{
  double alpha = 0.01;
  double tf, ts, dt;
  
  tf = fof_time(unit->mBank); /* frame counting */
  ts = systime();
  unit->mTframe = ts;
  dt = ts - tf;
  unit->mDtma = alpha * dt + (1.0-alpha) * unit->mDtma ;

  if( fabs(unit->mDtma - dt) > 1.0)
    unit->mDtma = dt;
        
    unit->mTcorr = round(unit->mDtma*1e4)*1e-4;
}
#endif

int manager_activate_clients(manager* mgr)
{
  int status;
  
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    status = dsp_client_activate(mgr->dsp[i]);
    if (status)
      return status;
  }
  return ctrl_client_activate(mgr->ctrl);
}

int manager_connect_clients(manager* mgr)
{
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    int status;
    status = jack_connect(mgr->ctrl->j_client,
			  jack_port_name(mgr->ctrl->port),
			  jack_port_name(mgr->dsp[i]->in_port));
    if (status)
      return status;
  }
#if 0
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    dsp_client* dsp = mgr->ctrl->dsp[i];
    for (int j = 0; j < mgr->ctrl->nchans; j++)
    {
      *status = jack_connect(dsp->j_client,
			     jack_port_name(dsp->out_port[j]),
			     jack_port_name(mix->in_port[j]));
      if (status)
	return status;
    }
  }
  
#endif
  return JFOFS_SUCCESS;
}
