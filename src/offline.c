#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sndfile.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "offline.h"

struct offline_job_s
{
  FofBank *fof_bank;
  SNDFILE *sf;
  SF_INFO sf_info;
  int n_chans;
  int block_size;
  float *ch_buf[MAX_CHANNELS];  /* per-channel render buffers */
  float *interleaved;           /* interleaved buffer for sndfile */
  uint64_t frame_count;
};

offline_job_t *offline_job_create(const char *path, const setup_t *setup,
                                  int block_size, int *status)
{
  offline_job_t *job = NULL;
  int n_chans;
  SF_INFO sf_info;

  *status = JFOFS_MEMORY_ERROR;

  job = calloc(1, sizeof(offline_job_t));
  if (job == NULL)
    return NULL;

  n_chans = fof_ModeToChannels(setup->mode);
  job->n_chans = n_chans;
  job->block_size = block_size;
  job->frame_count = 0;

  job->fof_bank = fof_newBank(setup->sample_rate, setup->mode,
                              setup->n_preallocate_fofs, block_size);
  if (job->fof_bank == NULL)
  {
    free(job);
    return NULL;
  }

  fof_set_trace_level(setup->fofs_trace_level);

  /* allocate per-channel buffers */
  for (int i = 0; i < n_chans; i++)
  {
    job->ch_buf[i] = calloc((size_t)block_size, sizeof(float));
    if (job->ch_buf[i] == NULL)
    {
      for (int j = 0; j < i; j++)
        free(job->ch_buf[j]);
      fof_freeBank(job->fof_bank);
      free(job);
      return NULL;
    }
  }

  /* interleaved scratch buffer for sndfile */
  job->interleaved = calloc((size_t)(block_size * n_chans), sizeof(float));
  if (job->interleaved == NULL)
  {
    for (int i = 0; i < n_chans; i++)
      free(job->ch_buf[i]);
    fof_freeBank(job->fof_bank);
    free(job);
    return NULL;
  }

  /* open output file */
  memset(&sf_info, 0, sizeof(sf_info));
  sf_info.samplerate = setup->sample_rate;
  sf_info.channels = n_chans;
  sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  job->sf_info = sf_info;
  job->sf = sf_open(path, SFM_WRITE, &job->sf_info);
  if (job->sf == NULL)
  {
    fprintf(stderr, "offline_job_create: sf_open failed: %s\n",
            sf_strerror(NULL));
    free(job->interleaved);
    for (int i = 0; i < n_chans; i++)
      free(job->ch_buf[i]);
    fof_freeBank(job->fof_bank);
    free(job);
    *status = JFOFS_FALIURE;
    return NULL;
  }

  *status = JFOFS_SUCCESS;
  return job;
}

int offline_job_add(offline_job_t *job, uint64_t time_us, const float *argv)
{
  return fof_add_v(job->fof_bank, time_us, argv);
}

int offline_job_process(offline_job_t *job)
{
  sf_count_t written;

  fof_next(job->fof_bank, (unsigned int)job->block_size, job->ch_buf);

  /* interleave per-channel buffers for sndfile */
  for (int f = 0; f < job->block_size; f++)
  {
    for (int c = 0; c < job->n_chans; c++)
    {
      job->interleaved[f * job->n_chans + c] = job->ch_buf[c][f];
    }
  }

  written = sf_writef_float(job->sf, job->interleaved, (sf_count_t)job->block_size);
  if (written != (sf_count_t)job->block_size)
  {
    fprintf(stderr, "offline_job_process: sf_writef_float wrote %lld of %d frames: %s\n",
            (long long)written, job->block_size, sf_strerror(job->sf));
    return JFOFS_FALIURE;
  }

  job->frame_count += (uint64_t)job->block_size;
  return JFOFS_SUCCESS;
}

void offline_job_close(offline_job_t *job)
{
  if (job == NULL)
    return;

  if (job->sf != NULL)
    sf_close(job->sf);

  fof_freeBank(job->fof_bank);

  free(job->interleaved);
  for (int i = 0; i < job->n_chans; i++)
    free(job->ch_buf[i]);

  free(job);
}
