#import "jfofs_dsp.h"
#import "jfofs_types.h"

int jfofs_dsp_process (jack_nframes_t nframes, void *arg);

jfofs_dsp* jfofs_dsp_new(jfofs_controller* cntl, int sample_rate, FofMode mode,
			 int* status)
{
  const char *client_name = "jfofs_dsp";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jfofs_dsp* dsp;

  *status = posix_memalign((void**) &dsp, CACHE_LINE_SIZE, sizeof(jfofs_dsp));
  if (dsp == NULL)
  {
    return NULL;
  }
  
  dsp->controller = cntl;
  dsp->n_fofs = 0;
  dsp->fof_bank = fof_newBank(sample_rate, mode);
  if (dsp->fof_bank == NULL)
  {
    *status = JFOFS_MEMORY;
    free(dsp);
    return NULL;
 } 

  dsp->j_client = jack_client_open(client_name, options, &status, server_name);
  if (dsp->j_client == NULL)
  {
    fof_freeBank(dsp->fof_bank);
    free(dsp);
    return NULL;
  }
   
  dsp->in_port = jack_port_register(controller->j_client, "input",
				    JACK_DEFAULT_AUDIO_TYPE,
				    JackPortIsInput, 0);
  
  for (int i = 0; i < cntl->nclients; i++)
  {
    char name[64];
    
    sprintf (name, "output_%d", i+1);
    dsp->out_port[i] = jack_port_register(controller->j_client, name,
				    JACK_DEFAULT_AUDIO_TYPE,
				    JackPortIsOutput, 0);  
  }
  jack_set_process_callback(dsp->j_client , jfofs_dsp_process,
			    (void*) dsp->j_client);
  return dsp;
}

void jfofs_dsp_add(jfofs_dsp* dsp, fof* fof)
{
  fof_add_v(fof_bank, fof->time, fof->argv);
  
  /* TODO: how to count fofs? 
   * Implement a counter in fofs.c
   */
}

int jfofs_dsp_process (jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t **buf;
  jfofs_dsp* dsp = (jfofs_dsp*) arg;

  for(int i = 0; i < dsp->n_ports; i++)
  {
    buf[i] = jack_port_get_buffer(dsp->port[i], nframes);
  }
  
  fof_next(dsp->fof_bank, nframes, buf);

  return 0;
}
