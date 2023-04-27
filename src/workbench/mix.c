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
