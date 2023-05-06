#include <stdlib.h>
#include <string.h>
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

manager* manager_new(setup* _setup, int *status)
{
  manager* mgr;

  *status = posix_memalign((void**) &mgr, CACHE_LINE_SIZE, sizeof(manager));

  if (mgr == NULL)
  {
    return NULL;
  }

  memcpy((char*) &(mgr->setup), (char*) _setup, sizeof(setup));
  mgr->ctrl = ctrl_client_new(&mgr->setup, status);
  mgr->q = mgr->ctrl->q;
  mgr->mix = mix_client_new(fof_ModeToChannels(mgr->setup.mode), status);
  
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    dsp_client* dsp = dsp_client_new(&mgr->setup, status);
    mgr->dsp[i] = dsp;
    mgr->ctrl->dsp[i] = dsp;
  }

  return mgr;
}

void manager_free(manager* mgr)
{
  ctrl_client_free(mgr->ctrl);
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    dsp_client_free(mgr->dsp[i]);
  }
  //mix_client_free(mgr->mix);
  free(mgr);
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

  status = mix_client_activate(mgr->mix);
  if (status)
      return status;
  
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    status = dsp_client_activate(mgr->dsp[i]);
    if (status)
      return status;
  }

  return ctrl_client_activate(mgr->ctrl);
}

int manager_deactivate_clients(manager* mgr)
{
  int status;

  status = ctrl_client_deactivate(mgr->ctrl);

  if (status)
      return status;
  
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    status = dsp_client_deactivate(mgr->dsp[i]);
    if (status)
      return status;
  }

  return mix_client_deactivate(mgr->mix);
}

int manager_connect_clients(manager* mgr)
{
  int status;
  
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    status = jack_connect(mgr->ctrl->j_client,
			  jack_port_name(mgr->ctrl->port),
			  jack_port_name(mgr->dsp[i]->in_port));
    if (status)
      return status;
  }

  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    dsp_client* dsp = mgr->dsp[i];
    for (int j = 0; j < dsp->n_chans; j++)
    {
      status = jack_connect(dsp->j_client,
			    jack_port_name(dsp->out_port[j]),
			    jack_port_name(mgr->mix->in_port[j]));
      if (status)
	return status;
    }
  }
  
  return JFOFS_SUCCESS;
}
