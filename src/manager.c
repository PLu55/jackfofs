#include <jack/jack.h>

#include "process_queue.h"
#include "controller.h"
#include "jfof.h"
#include "jfof_private.h"

fof_queue* fof_queue_new(int sample_rate, int n_slots, int slot_size,
			 int n_free_chunks, int chunk_size, int* status);

jfofs_controller* jfofs_controller_new(FofsMode mode, int nclients,
				       int nchans, int* status);

typedef struct manager_s manager;

struct manager_s
{
  ctrl_client* ctrl;
  dsp_client* dsp[MAX_DSP_CLIENTS];
  mix_client* mix;
  setup setup;
};

manager* manager_new(void)
{
  manager* mgr;

  int status;
  jack_nframes_t n ,m;
  fof _fof;
  jack_nframes_t sample_rate;
  jack_nframes_t buf_size;
  jack_time_t t0;

  *status = posix_memalign((void**) &mgr, CACHE_LINE_SIZE, sizeof(manager));

  if (mgr == NULL)
  {
    return NULL;
  }

  mgr->setup.mode = FOF_MONO;
  mgr->setup.n_clients = 1;
  mgr->setup.n_preallocate_fofs = 1024;
  mgr->setup.>n_slots = 64;
  mgr->setup.>n_free_chunks = 128;
  mgr->setup.chunk_size = 256;
  
  mgr->ctrl = ctrl_client_new(&mgr->setup, &status);
  
  sample_rate = jack_get_sample_rate(ctrl->j_client);
  buf_size = jack_get_buffer_size(ctrl->j_client);

  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    mgr->dsp[i] = dsp_client_new(FofMode mode, int n_fofs_per_client, int* status)
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

int manager_client_connect(manager* mgr)
{
  for (int i = 0; i < ctrl->n_clients; i++)
  {
    status = jack_connect(mgr->ctrl->j_client,
			  jack_port_name(mgr->ctrl->port),
			  jack_port_name(mgr->dsp[i]->in_port));
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
}
