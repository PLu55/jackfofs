#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <unity/unity.h>
#include <sndfile.h>
#include <fofs.h>

#include "jfofs_offline.h"

#define OFFLINE_TMP_MONO   "/tmp/jfofs_test_offline_mono.wav"
#define OFFLINE_TMP_STEREO "/tmp/jfofs_test_offline_stereo.wav"
#define OFFLINE_TMP_MULTI  "/tmp/jfofs_test_offline_multi.wav"

/*
 * jfofs_offline_setup_t - parameters for an offline rendering job.
 */
typedef struct jfofs_offline_setup_s
{
  int sample_rate;        /* samples per second, e.g. 48000        */
  int mode;               /* FOF mode (defines channel count)       */
  int n_preallocate_fofs; /* number of pre-allocated FOF voices     */
  int fofs_trace_level;   /* trace verbosity level for libfofs      */
} jfofs_offline_setup_t;

/* Build a minimal jfofs_offline_setup_t for offline tests. */
static jfofs_offline_setup_t make_offline_setup(int mode, int sample_rate)
{
  jfofs_offline_setup_t s;
  memset(&s, 0, sizeof(s));
  s.mode               = mode;
  s.fofs_trace_level = 0;
  s.n_preallocate_fofs = 64;
  s.sample_rate = sample_rate;
  return s;
}

/* Populate argv with sensible default FOF parameters. */
static void default_argv(float *argv)
{
  argv[FOF_ARG_ampl]   = 1.0f;
  argv[FOF_ARG_freq]   = 100.0f;
  argv[FOF_ARG_gliss]  = 0.0f;
  argv[FOF_ARG_phi]    = 0.0f;
  argv[FOF_ARG_beta]   = 0.1f;
  argv[FOF_ARG_alpha]  = 13.816f; /* ~1 s duration */
  argv[FOF_ARG_amin]   = 0.001f;
  argv[FOF_ARG_cutoff] = 0.002f;
  argv[FOF_ARG_pan1]   = 0.0f;
  argv[FOF_ARG_pan2]   = 0.0f;
  argv[FOF_ARG_pan3]   = 0.0f;
}

/* ---- test_offline_create_close ------------------------------------------ */

/* Basic create/close round-trip: file must be created and no crash on close. */
void test_offline_create_close(void)
{
  jfofs_offline_setup_t setup = make_offline_setup(FOF_MONO, 44100);
  int status;

  jfofs_offline_job_t *job = jfofs_offline_create(OFFLINE_TMP_MONO,
                                                  setup.sample_rate,
                                                  setup.mode,
                                                  setup.n_preallocate_fofs,
                                                  setup.fofs_trace_level,
                                                  256,
                                                  &status);

  TEST_ASSERT_NOT_NULL(job);
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);

  jfofs_offline_close(job);

  /* output file must exist after close */
  TEST_ASSERT_EQUAL_INT(0, access(OFFLINE_TMP_MONO, F_OK));
  unlink(OFFLINE_TMP_MONO);
}

/* ---- test_offline_invalid_path ------------------------------------------ */

/* Creating a job with an unwritable path must fail gracefully. */
void test_offline_invalid_path(void)
{
  jfofs_offline_setup_t setup = make_offline_setup(FOF_MONO, 44100);
  int status = JFOFS_SUCCESS;

  jfofs_offline_job_t *job = jfofs_offline_create("/nonexistent_dir/out.wav",
                                                  setup.sample_rate,
                                                  setup.mode,
                                                  setup.n_preallocate_fofs,
                                                  setup.fofs_trace_level,
                                                  256,
                                                  &status);

  TEST_ASSERT_NULL(job);
  TEST_ASSERT_NOT_EQUAL(JFOFS_SUCCESS, status);
}

/* ---- test_offline_mono_renders ------------------------------------------ */

/*
 * Mono render: schedule one FOF at t=0, process 20 blocks and verify
 * that the output WAV has the correct metadata and non-zero audio samples.
 */
void test_offline_mono_renders(void)
{
  const int sample_rate = 44100;
  const int block_size = 256;
  const int n_blocks = 20;
  jfofs_offline_setup_t setup = make_offline_setup(FOF_MONO, sample_rate);
  int status;
  float argv[FOF_NUMARGS];
  default_argv(argv);

  jfofs_offline_job_t *job = jfofs_offline_create(OFFLINE_TMP_MONO,
                                                  setup.sample_rate,
                                                  setup.mode,
                                                  setup.n_preallocate_fofs,
                                                  setup.fofs_trace_level,
                                                  block_size,
                                                  &status);
  TEST_ASSERT_NOT_NULL(job);
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);

  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, jfofs_offline_add(job, 0UL, argv));

  for (int b = 0; b < n_blocks; b++)
    TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, jfofs_offline_process(job));

  jfofs_offline_close(job);

  /* read back and verify metadata */
  SF_INFO info;
  memset(&info, 0, sizeof(info));
  SNDFILE *sf = sf_open(OFFLINE_TMP_MONO, SFM_READ, &info);
  TEST_ASSERT_NOT_NULL(sf);
  TEST_ASSERT_EQUAL_INT(sample_rate, info.samplerate);
  TEST_ASSERT_EQUAL_INT(1, info.channels);
  TEST_ASSERT_EQUAL_INT64((sf_count_t)(block_size * n_blocks), info.frames);

  /* verify at least some non-zero audio was written */
  float buf[256];
  sf_count_t rd = sf_readf_float(sf, buf, 256);
  TEST_ASSERT_EQUAL_INT64(256, rd);

  float sum = 0.0f;
  for (int i = 0; i < 256; i++)
    sum += fabsf(buf[i]);
  TEST_ASSERT_TRUE(sum > 0.0f);

  sf_close(sf);
  unlink(OFFLINE_TMP_MONO);
}

/* ---- test_offline_stereo_renders ---------------------------------------- */

/*
 * Stereo render: verify channel count is 2 and total frame count matches.
 */
void test_offline_stereo_renders(void)
{
  const int sample_rate = 48000;
  const int block_size  = 512;
  const int n_blocks    = 10;
  jfofs_offline_setup_t setup = make_offline_setup(FOF_STEREO, sample_rate);
  int status;
  float argv[FOF_NUMARGS];
  default_argv(argv);

  jfofs_offline_job_t *job = jfofs_offline_create(OFFLINE_TMP_STEREO,
                                                  setup.sample_rate,
                                                  setup.mode,
                                                  setup.n_preallocate_fofs,
                                                  setup.fofs_trace_level,
                                                  block_size,
                                                  &status);
  TEST_ASSERT_NOT_NULL(job);
  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, status);

  TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, jfofs_offline_add(job, 0UL, argv));

  for (int b = 0; b < n_blocks; b++)
    TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, jfofs_offline_process(job));

  jfofs_offline_close(job);

  SF_INFO info;
  memset(&info, 0, sizeof(info));
  SNDFILE *sf = sf_open(OFFLINE_TMP_STEREO, SFM_READ, &info);
  TEST_ASSERT_NOT_NULL(sf);
  TEST_ASSERT_EQUAL_INT(sample_rate, info.samplerate);
  TEST_ASSERT_EQUAL_INT(2, info.channels);
  TEST_ASSERT_EQUAL_INT64((sf_count_t)(block_size * n_blocks), info.frames);

  sf_close(sf);
  unlink(OFFLINE_TMP_STEREO);
}

/* ---- test_offline_multiple_fofs ----------------------------------------- */

/*
 * Schedule several FOFs staggered in time and verify the job completes
 * without error and produces the expected number of frames.
 */
void test_offline_multiple_fofs(void)
{
  const int sample_rate = 44100;
  const int block_size = 256;
  const int n_blocks = 40;
  jfofs_offline_setup_t setup = make_offline_setup(FOF_MONO, sample_rate);
  int status;
  float argv[FOF_NUMARGS];
  default_argv(argv);

  jfofs_offline_job_t *job = jfofs_offline_create(OFFLINE_TMP_MULTI,
                                                  setup.sample_rate,
                                                  setup.mode,
                                                  setup.n_preallocate_fofs,
                                                  setup.fofs_trace_level,
                                                  block_size,
                                                  &status);
  TEST_ASSERT_NOT_NULL(job);

  /* schedule 5 FOFs 50 ms apart */
  for (int k = 0; k < 5; k++)
  {
    uint64_t t = (uint64_t)k * 50000UL;
    int s = jfofs_offline_add(job, t, argv);
    TEST_ASSERT_TRUE(s == JFOFS_SUCCESS || s == JFOFS_FOF_LATE_WARNING);
  }

  for (int b = 0; b < n_blocks; b++)
    TEST_ASSERT_EQUAL_INT(JFOFS_SUCCESS, jfofs_offline_process(job));

  jfofs_offline_close(job);

  SF_INFO info;
  memset(&info, 0, sizeof(info));
  SNDFILE *sf = sf_open(OFFLINE_TMP_MULTI, SFM_READ, &info);
  TEST_ASSERT_NOT_NULL(sf);
  TEST_ASSERT_EQUAL_INT64((sf_count_t)(block_size * n_blocks), info.frames);
  sf_close(sf);
  unlink(OFFLINE_TMP_MULTI);
}
