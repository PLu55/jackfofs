#ifndef __SIGNAL_TESTER_H__
#define __SIGNAL_TESTER_H__

#include <jack/jack.h>
#include <stdint.h>

typedef struct signal_tester_client_s signal_tester_client_t;

struct signal_tester_client_s
{
  jack_client_t* j_client;
  jack_port_t* in_port;
  uint64_t n;
  uint64_t n_frames;
  float sum;
  float min;
  float max;
};

signal_tester_client_t* signal_tester_client_new(int* status);
void signal_tester_client_set_nframes(signal_tester_client_t* stc, uint64_t n);
void signal_tester_client_free(signal_tester_client_t* stc);
float signal_tester_client_rms(signal_tester_client_t* stc);
void signal_tester_client_activate(signal_tester_client_t* stc);
void signal_tester_client_deactivate(signal_tester_client_t* stc);
void signal_tester_client_reset(signal_tester_client_t* stc);


#endif
