#ifndef __JFOFS_DSP_H__
#define __JFOFS_DSP_H__

#include <jack/jack.h>
#include <fofs.h>

#include "jfofs.h"
#include "jfofs_types.h"
#include "jfofs_private.h"

typedef struct dsp_client_s dsp_client_t;

struct dsp_client_s
{
  int n_fofs;
  FofBank* fof_bank;
  int n_chans;
  jack_client_t* j_client;
  jack_port_t* in_port;
  jack_port_t* out_port[MAX_CHANNELS];
  void* pad[3];
};

static inline int dsp_client_add(dsp_client_t* dsp, fof_t* fof)
{
  return fof_add_v(dsp->fof_bank, fof->time_us, fof->argv);
}

dsp_client_t* dsp_client_new(setup_t* _setup, int index, int* status);
int dsp_client_add(dsp_client_t* dsp, fof_t* fof);
int dsp_client_activate(dsp_client_t* dsp);
int dsp_client_deactivate(dsp_client_t* dsp);
void dsp_client_free(dsp_client_t* dsp);
uint64_t dsp_get_next_frame(dsp_client_t* dsp);
void dsp_set_next_frame(dsp_client_t* dsp, uint64_t n);

#endif
