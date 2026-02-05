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

int connect_ports(mix_client_t *mix, sin_gen_t *sgen, signal_tester_client_t **stc,
                  int n_chans);

static int jack_connect_retry(jack_client_t *client,
                              const char *src,
                              const char *dst,
                              int max_attempts,
                              useconds_t sleep_us)
{
  int rc = 0;

  for (int attempt = 0; attempt < max_attempts; attempt++)
  {
    rc = jack_connect(client, src, dst);
    if (rc == 0 || rc == 17) /* EEXIST */
      return 0;
    usleep(sleep_us);
  }

  return rc;
}

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
  mix_client_t *mix;
  sin_gen_t *sgen;
  signal_tester_client_t *stc[MAX_CHANNELS];
  float mins[MAX_CHANNELS];
  float maxs[MAX_CHANNELS];
  float rmss[MAX_CHANNELS];
#ifdef VERBOSE_ENABLE
  uint64_t ns[MAX_CHANNELS];
#endif
  double freq;
  double ampl;
  int status;
  shmem_t *shmem;
  fof_queue_t *q;
  setup_t setup;

  freq = 1000.0;
  ampl = 0.01;

  setup.n_slots = 32;
  setup.n_max_fofs = 1024;

  sgen = sin_gen_new(freq, ampl, &status);
  TEST_ASSERT_NOT_NULL(sgen);

  setup.sample_rate = jack_get_sample_rate(sgen->j_client);
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

  for (int i = 0; i < n_chans; i++)
  {
    stc[i] = signal_tester_client_new(&status);
    TEST_ASSERT_NOT_NULL(stc[i]);
    signal_tester_client_set_nframes(stc[i], (uint64_t)(setup.sample_rate * 0.5));
  }

  TEST_ASSERT_EQUAL_INT(0, sin_gen_activate(sgen));
  TEST_ASSERT_EQUAL_INT(0, mix_client_activate(mix));

  for (int i = 0; i < n_chans; i++)
    signal_tester_client_activate(stc[i]);

  usleep(100 * 1000);

  TEST_ASSERT_EQUAL_INT(0, connect_ports(mix, sgen, stc, n_chans));

  sleep(1);
  for (int i = 0; i < n_chans; i++)
    signal_tester_client_deactivate(stc[i]);
  sin_gen_deactivate(sgen);
  mix_client_deactivate(mix);

  for (int i = 0; i < n_chans; i++)
  {
    mins[i] = stc[i]->min;
    maxs[i] = stc[i]->max;
    rmss[i] = signal_tester_client_rms(stc[i]);
#ifdef VERBOSE_ENABLE
    ns[i] = stc[i]->n;
#endif
    signal_tester_client_free(stc[i]);
  }
  sin_gen_free(sgen);
  mix_client_free(mix);
  shmem_unmap(shmem);
  shmem_unlink(shmem);

  for (int i = 0; i < n_chans; i++)
  {
#ifdef VERBOSE_ENABLE
    printf("ch%d min: %f max: %f RMS: %f n: %ld\n", i, mins[i], maxs[i], rmss[i], ns[i]);
#endif
    TEST_ASSERT_FLOAT_WITHIN(1e-6, ampl, maxs[i]);
    TEST_ASSERT_FLOAT_WITHIN(1e-6, ampl, -mins[i]);
    TEST_ASSERT_FLOAT_WITHIN(1e-3, ampl * M_SQRT1_2, rmss[i]);
  }
}

int connect_ports(mix_client_t *mix, sin_gen_t *sgen, signal_tester_client_t **stc,
                  int n_chans)
{
  int status = 0;

  for (int i = 0; i < n_chans; i++)
  {
    const char *sgen_out = jack_port_name(sgen->out_port);
    const char *mix_in = jack_port_name(mix->in_port[i]);
    const char *mix_out = jack_port_name(mix->out_port[i]);
    const char *stc_in = jack_port_name(stc[i]->in_port);

    int rc = jack_connect_retry(sgen->j_client, sgen_out, mix_in, 50, 10 * 1000);
    if (rc != 0)
    {
      fprintf(stderr, "jack_connect failed rc=%d src=%s dst=%s\n", rc,
              sgen_out ? sgen_out : "(null)", mix_in ? mix_in : "(null)");
      return rc;
    }

    rc = jack_connect_retry(mix->j_client, mix_out, stc_in, 50, 10 * 1000);
    if (rc != 0)
    {
      fprintf(stderr, "jack_connect failed rc=%d src=%s dst=%s\n", rc,
              mix_out ? mix_out : "(null)", stc_in ? stc_in : "(null)");
      return rc;
    }
  }

  return status;
}
