#ifndef __JFOFS_PRIVATE_H__
#define __JFOFS_PRIVATE_H__

#include <semaphore.h>
#include <jack/jack.h>
#include <fofs.h>

#include "setup.h"

#define CACHE_LINE_SIZE 64UL
#define MAX_CHANNELS 8
#define MAX_DSP_CLIENTS 8
#define MAX_SLOTS 128
#define SHMEM_NAME "jfofs_shm"

typedef struct ctrl_client_s ctrl_client_t;
typedef struct dsp_client_s dsp_client_t;
typedef struct mix_client_s mix_client_t;
typedef struct fof_queue_s fof_queue_t;
typedef struct setup_s setup_t;
typedef struct shmem_s shmem_t;
typedef struct fof_s fof_t;

struct fof_s
{
  fof_t* next;
  uint64_t time_us;         /* time in microsecunds */
  float argv[FOF_NUMARGS];
};

static inline jack_nframes_t jfofs_time_to_nframes(uint64_t t, int sample_rate)
{
  return t * sample_rate / 1000000ULL;
}

static inline uint64_t jfofs_nframes_to_time(uint64_t n, int sample_rate)
{
  return n * 1000000ULL / sample_rate;
}

#endif
