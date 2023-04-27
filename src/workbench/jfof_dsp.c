#import "jfofs_dsp.h"
#import "jfofs_types.h"

jfofs_dsp_data_init(jfofs_dsp_data* d, jfofs_controller* controller,
		    int sample_rate, FofMode mode)
{
  d->controller = controller;
  d->n_fofs = 0;
  d->fof_bank = fof_newBank(sample_rate, mode);
  /* TODO: allocate ports */
}

void jfofs_dsp_add(jfofs_dsp_data* d, fof* _fof)
{
  fof_add_v(fof_bank, _fof->time, _fof->argv);
  /* TODO: how to count fofs? 
   * Implement a counter in fofs.c
   */
}

int jfofs_dsp_process (jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t **buf;
  jfofs_dsp_data* d = (jfofs_dsp_data*) arg;

  for(int i = 0; i < d->n_ports; i++)
  {
    buf[i] = jack_port_get_buffer(d->port[i], nframes);
  }
  
  /* lock is needed to write to d->controller */
  fof_next(d->fof_bank, nframes, buf);

  return 0;
}
