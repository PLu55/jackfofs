#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <jack/jack.h>

#include "jfofs_types.h"
#include "mixer.h"

int test_mixer()
{
  jfofs_status status;
  int stat = 0;
  sin_gen* mixer = mixer_new(1000.0, 0.1, 1, &status);

  if (mixer == NULL)
  {
    fprintf(stderr, "test_mixer: error code: %d\n", status);
    exit(EXIT_FAILURE);
  }
  
  
  //stat = jack_connect(sgen->j_client, jack_port_name(sgen->port[0]),
  //			  "system:playback_1");
  if (stat != 0)
  {
    fprintf(stderr, "test_sin_gen: error connecting ports, code: %d\n", stat);
    exit(EXIT_FAILURE);
  }
  
}
