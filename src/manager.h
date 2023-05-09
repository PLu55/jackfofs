#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "jfofs_private.h"
#include "fof_queue.h"
#include "ctrl_client.h"
#include "dsp_client.h"
#include "mix_client.h"

typedef struct manager_s manager;

struct manager_s
{
  ctrl_client* ctrl;
  dsp_client* dsp[MAX_DSP_CLIENTS];
  mix_client* mix;
  fof_queue* q;
  setup setup;
  shm_t* shm;
};

static inline void manager_add(manager* mgr, fof* _fof)
{
  fof_queue_add(mgr->q, _fof);
}

manager* manager_create(setup* _setup, int *status);
manager* manager_new(setup* _setup, int *status);
void manager_free(manager* mgr);
int manager_activate_clients(manager* mgr);
int manager_deactivate_clients(manager* mgr);
int manager_connect_clients(manager* mgr);
//void manager_add(manager* mgr, fof* _fof);
int manager_setup_ipc(manager* mgr);
void manager_ipc_loop(manager* mgr);

#endif
