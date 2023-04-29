#ifndef __SIGNAL_TESTER_H__
#define __SIGNAL_TESTER_H__

typedef struct signal_tester_client_s signal_tester_client;

struct signal_tester_client_s
{
  jack_client_t* j_client;
  jack_port_t* in_port;
  uint64_t n;
  uint64_t m;
  float sum;
  float min;
  float max;
};

signal_tester_client* signal_tester_client_new(int* status);
float signal_tester_client_rms(signal_tester_client* stc);
void signal_tester_client_activate(signal_tester_client* stc);
void signal_tester_client_deactivate(signal_tester_client* stc);

#endif
