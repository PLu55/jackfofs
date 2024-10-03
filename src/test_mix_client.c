#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "mix_client.h"
#include "signal_tester_client.h"
#include "sin_gen.h"
#include "test_util.h"
#include "shmem.h"
#include "fof_queue.h"

void test_mix_client_with_nchans(int n_chans);

int connect_ports(mix_client_t* mix, sin_gen_t* sgen, signal_tester_client_t* stc,
		  int n_chans);

void test_mix_client(void)
{
  test_mix_client_with_nchans(1);
  sleep(2);
  test_mix_client_with_nchans(2);
  sleep(2);
  test_mix_client_with_nchans(4);
  sleep(2);
  test_mix_client_with_nchans(8);
}

void test_mix_client_with_nchans(int n_chans)
{
  mix_client_t* mix;
  sin_gen_t* sgen;
  signal_tester_client_t* stc;
  double freq;
  double ampl;
  int status;
  shmem_t* shmem;
  fof_queue_t* q;
  setup_t setup;
  
  freq = 1000.0;
  ampl = 0.01;

  setup.n_slots = 32;
  setup.n_max_fofs = 1024;

  sgen = sin_gen_new(freq, ampl, &status);
  TEST_ASSERT_NOT_NULL(sgen);

  //setup.sample_rate = jack_get_sample_rate(sgen->j_client);
  setup.sample_rate = 48000;
  setup.max_buffer_size = jack_get_buffer_size(sgen->j_client);
  
  shmem = shmem_create(&setup, &status);
  TEST_ASSERT_NOT_NULL(shmem);
  
  q = &(shmem->q);

  fof_queue_init(q, &setup);

  mix = mix_client_new(n_chans, q, &status);
  TEST_ASSERT_NOT_NULL(mix);
  TEST_ASSERT_EQUAL_INT(n_chans, mix->n_chans);
  TEST_ASSERT_NOT_NULL(mix->in_port[0]);
  TEST_ASSERT_NOT_NULL(mix->out_port[0]);

  stc = signal_tester_client_new(&status);
  TEST_ASSERT_NOT_NULL(stc);
  
  signal_tester_client_set_nframes(stc, (uint64_t)(setup.sample_rate * 0.5));

  sin_gen_activate(sgen);
  mix_client_activate(mix);
  signal_tester_client_activate(stc);
  
  connect_ports(mix, sgen, stc, n_chans);
  
  sleep(1);
  signal_tester_client_deactivate(stc);
  sin_gen_deactivate(sgen);
  mix_client_deactivate(mix);

  printf("min: %f max: %f RMS: %f n: %ld\n", stc->min, stc->max,
	 signal_tester_client_rms(stc), stc->n);
  TEST_ASSERT_FLOAT_WITHIN(1e-6, ampl * n_chans , stc->max);
  TEST_ASSERT_FLOAT_WITHIN(1e-6, ampl * n_chans, -stc->min);
  TEST_ASSERT_FLOAT_WITHIN(1e-3, ampl * M_SQRT1_2 * n_chans ,
  			   signal_tester_client_rms(stc));
  signal_tester_client_free(stc);
  sin_gen_free(sgen);
  mix_client_free(mix);
}

int connect_ports(mix_client_t* mix, sin_gen_t* sgen, signal_tester_client_t* stc,
		  int n_chans)
{
  int status = 0;
  
  for (int i = 0; i < n_chans; i++)
  {
    status &= jack_connect(sgen->j_client,
			  jack_port_name(sgen->out_port),
			  jack_port_name(mix->in_port[i]));
    status &= jack_connect(mix->j_client,
		 jack_port_name(mix->out_port[i]),
		 jack_port_name(stc->in_port));
  }
  return status;
}

