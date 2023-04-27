#include <jack/jack.h>

#define FOFS_JACK_MAX_CHAN 8
#define FOFS_JACK_MAX_CLIENTS 64

typedef struct fofs_jack_client_s fofs_jack_client;

struct fofs_jack_client_s
{
  FofBank fof_bank;
  int nchans;
  FofMode mode;
  jack_client_t j_client;
  jack_port_t *port[FOFS_JACK_CLIENT_MAX_CHAN];
};

typedef struct fofs_jack_mixer_s fofs_jack_mixer;

fofs_jack_mixer_s
{
  jack_port_t **in_port;
  jack_port_t **out_port;
};

typedef struct fofs_jack_controller_s fofs_jack_controller;

fofs_jack_controller_s
{
  int sample_rate;
  int nclients;
  FofMode mode;
  jack_client_t j_client;
  fofs_jack_mixer* mixer;
  fofs_jack_client* client[FOFS_JACK_MAX_CLIENTS];
};

