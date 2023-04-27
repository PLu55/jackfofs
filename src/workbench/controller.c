#include <stdlib.h>
#include <jack/jack.h>

#include <fofs.h>

#include "controller.h"
#include "fofs_jack_client.h"

// jack_set_buffer_size_callback()
// jack_set_sample_rate_callback()


  
fofs_jack_controller* fofs_jack_controller_new(int nclients, int nchans,
					       jfofs_status* status)
{
  int nchans = fofs_jack_mode_to_nchans(mode);
  const char *client_name = "jfofs_controller";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t jstatus;
  
  jfofs_controller* controller = (jfofs_controller*) malloc(sizeof(jfofs_controller));
  if (controller = NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  
  controller->nclients = nclients;
  controller->nchans = nchans;
  controller->j_client = jack_client_open(client_name, options, &jstatus,
					  server_name);
  if (controller->j_client == NULL)
  {
    *status = JFOFS_JACK_ERROR | jstatus;
    return NULL;
  }
  
  controller->sample_rate = jack_get_sample_rate();
  
  
  return controller;
}

wow()
{
  
}
