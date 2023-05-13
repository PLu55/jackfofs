#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "jfofs_private.h"
#include "shmem.h"
#include "fof_queue.h"
#include "ctrl_client.h"
#include "dsp_client.h"
#include "mix_client.h"

typedef struct manager_s manager_t;

struct manager_s
{
  ctrl_client_t* ctrl;
  dsp_client_t* dsp[MAX_DSP_CLIENTS];
  mix_client_t* mix;
  fof_queue_t* q;
  setup_t setup;
  shmem_t* shmem;
};

static inline void manager_add(manager_t* mgr, jfofs_time_t time_us, float* argv)
{
  fof_queue_add(mgr->q, time_us, argv);
}

manager_t* manager_create(setup_t* setup, int *status);
manager_t* manager_new(shmem_t* shmem, setup_t* setup, int *status);
void manager_free(manager_t* mgr);
int manager_activate_clients(manager_t* mgr);
int manager_deactivate_clients(manager_t* mgr);
int manager_connect_clients(manager_t* mgr);
//void manager_add(manager_t* mgr, fof_t* _fof);
int manager_create_shmem(manager_t* mgr);

#endif
