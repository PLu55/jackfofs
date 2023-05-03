#include <jack/jack.h>

#include "jfofs_private.h"
#include "controller.h"
#include "mix.h"

mix_client* mix_client_new(int* status)
{
  mix_client* mc;
  const char *client_name = "jfofs_mix";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  
  *status = posix_memalign((void**) &mc, CACHE_LINE_SIZE, sizeof(mix_client));
  if (mc == NULL)
    return NULL;

  mc->j_client = jack_client_open(client_name, options, status, server_name);
  if (mc->j_client == NULL)
  {
    free(mc);
    return NULL;
  }
  
  return mc;
}


int mix_client_activate(mix_client* mix)
{
  return jack_activate(mix->j_client);
}

int mix_client_deactivate(mix_client* mix)
{
  return jack_deactivate(mix->j_client);
}

void mix_client_free(mix_client* mix)
{
  mix_client_deactivate(mix);
  jack_client_close(mix->j_client);
  free(mix);
}
