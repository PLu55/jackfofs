#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h> /* debug */
#include <jack/jack.h>
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <semaphore.h>

#include "manager.h"
#include "shmem.h"
#include "fof_queue.h"
#include "ctrl_client.h"
#include "dsp_client.h"
#include "mix_client.h"
#include "jfofs.h"
#include "jfofs_private.h"

// jack_set_buffer_size_callback()
// jack_set_sample_rate_callback()

manager_t* manager_create(setup_t* setup, int *status)
{
  manager_t* mgr;
  shmem_t* shmem;
  
  shmem = shmem_create(setup, status);
  if (shmem == NULL)
  {
    return NULL;
  }
  
  mgr = manager_new(shmem, setup, status);

  if (mgr == NULL)
  {
    *status = JFOFS_MEMORY_ERROR;
    return NULL;
  }

  *status = manager_activate_clients(mgr);

  if (*status != 0)
  {
    manager_free(mgr);
    return NULL;
  }

  *status = manager_connect_clients(mgr);
  
  if (*status != 0)
  {
    manager_free(mgr);
    return NULL;
  }
  
  return mgr;
}

manager_t* manager_new(shmem_t* shmem, setup_t* setup, int *status)
{
  manager_t* mgr;

  *status = posix_memalign((void**) &mgr, CACHE_LINE_SIZE, sizeof(manager_t));

  if (mgr == NULL)
  {
    return NULL;
  }

  memset((char*) mgr, 0, sizeof(manager_t));
  memcpy((char*) &(mgr->setup), (char*) setup, sizeof(setup_t));

  mgr->shmem = shmem;
  mgr->q = &(shmem->q);
  mgr->ctrl = ctrl_client_new(setup, mgr->q, status);
  mgr->mix = mix_client_new(fof_ModeToChannels(setup->mode), mgr->q, status);
  mgr->mix->q = mgr->q;
  
  setup->sample_rate = jack_get_sample_rate(mgr->ctrl->j_client);
  setup->max_buffer_size = jack_get_buffer_size(mgr->ctrl->j_client);

  fof_queue_init(mgr->q, setup);
  
  printf("manager: setup.fof_trace_level: %d\n", setup->fofs_trace_level)
  ;
  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    dsp_client_t* dsp = dsp_client_new(setup, i, status);
    mgr->dsp[i] = dsp;
    mgr->ctrl->dsp[i] = dsp;
  }

  return mgr;
}

void manager_free(manager_t* mgr)
{
  //shm_unlink(SHMEM_NAME);
  if (mgr->ctrl != NULL)
    ctrl_client_free(mgr->ctrl);

  for (int i = 0; i < mgr->setup.n_clients; i++)
  {
    if (mgr->dsp[i] != NULL) 
      dsp_client_free(mgr->dsp[i]);
  }
  if (mgr->mix != NULL)
    mix_client_free(mgr->mix);
  
  shm_unlink(SHMEM_NAME);
  free(mgr);
}

inline jfofs_time_t systime()
{
  struct timespec tspec;

  clock_gettime(CLOCK_REALTIME, &tspec);
  return (jfofs_time_t) tspec.tv_sec * TIME_1  + (jfofs_time_t) tspec.tv_nsec * 1000;
}

int manager_activate_clients(manager_t* mgr)
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

int manager_deactivate_clients(manager_t* mgr)
{
  int status;

  status = ctrl_client_deactivate(mgr->ctrl) ;

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

int manager_connect_clients(manager_t* mgr)
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
    dsp_client_t* dsp = mgr->dsp[i];

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

/* Junk ************************************************************************/
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
