#ifndef __JFOFS_PRIVATE_H__
#define __JFOFS_PRIVATE_H__

#include <semaphore.h>
#include <jack/jack.h>
#include <fofs.h>

#include "jfofs.h"

#define CACHE_LINE_SIZE 64UL
#define MAX_CHANNELS 8
#define MAX_DSP_CLIENTS 8
#define SHM_NAME "jfofs_shm"

typedef struct ctrl_client_s ctrl_client;
typedef struct dsp_client_s dsp_client;
typedef struct mix_client_s mix_client;
typedef struct fof_queue_s fof_queue;
typedef struct chunk_s chunk;
typedef struct setup_s setup;
typedef struct shm_s shm_t;

struct fof_s
{
  uint64_t time_us;         /* time in microsecunds */
  float argv[FOF_NUMARGS];
  void* pad[1];
};

struct setup_s
{
  FofMode mode;              /* type of fof, defines the number of channels */
  int n_clients;             /* number of parallel dsp clients */  
  int n_preallocate_fofs;    /* number of pre allocated fofs in each client */
  int n_slots;               /* number of slots in circular fof buffer */
  int n_free_chunks;         /* number of fof chunks to allocate */
  int chunk_size;            /* number of fofs per chunk */
};

struct shm_s
{
  sem_t sem;
  fof fof;
};

static inline jack_nframes_t jfofs_time_us_to_nframe(uint64_t t, int sample_rate)
{
  return t * sample_rate / 1000000ULL;
}

static inline uint64_t jfofs_nframes_to_time_us(uint64_t n, int sample_rate)
{
  return n * 1000000ULL / sample_rate;
}

#endif
