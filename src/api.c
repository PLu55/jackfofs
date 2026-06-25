#include <fcntl.h>    /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include <jack/transport.h>

#include "config.h"
#include "jfofs.h"
#include "jfofs_types.h"
#include "jfofs_private.h"
#include "api.h"
#include "shmem.h"
#include "statistics.h"
#include "test_util.h"

const char *jfofs_version(void)
{
  return PROJECT_NAME_VER;
}

jfofs_t *jfofs_new(int *status, shmem_t *shmem)
{
  jfofs_t *jfofs = NULL;
  jack_status_t jstatus;

  *status = posix_memalign((void **)&jfofs, CACHE_LINE_SIZE, sizeof(jfofs_t));
  if (*status != 0 || jfofs == NULL)
  {
    *status = JFOFS_MEMORY_ERROR;
    return NULL;
  }

  jfofs->j_client = jack_client_open("jfofs_api", JackNullOption, &jstatus, NULL);

  if (jfofs->j_client == NULL)
  {
    *status = JFOFS_JACK_ERROR_MASK | jstatus;
    return NULL;
  }

  jack_activate(jfofs->j_client);

  if (shmem != NULL)
  {
    jfofs->is_shmem_linked = false;
    fprintf(stderr, "jfofs_new: shared memory is given! %p\n", (void *)shmem);
    jfofs->shmem = shmem;
    jfofs->shmem->reference_cnt++;
  }
  else
  {
    jfofs->is_shmem_linked = true;
    fprintf(stderr, "jfofs_new: shared memory is linked!\n");
    jfofs->shmem = shmem_link(status);

    if (jfofs->shmem == NULL)
    {
      if (*status == JFOFS_SHM_MAP_ERROR)
      {
        fprintf(stderr, "Error jfofs_new: can't map shared memory!\n");
      }
      else
      {
        perror(strerror(errno));
        *status = JFOFS_SHM_ERROR;
      }
      return NULL;
    }
  }

  *status = JFOFS_SUCCESS;
  return jfofs;
}

void jfofs_free(jfofs_t *jfofs)
{
  jack_deactivate(jfofs->j_client);
  jack_client_close(jfofs->j_client);
  if (jfofs->is_shmem_linked)
    shmem_unmap(jfofs->shmem);
  else
    jfofs->shmem->reference_cnt--;
  free(jfofs);
}

int jfofs_add(jfofs_t *jfofs, uint64_t time_us, float ampl, float freq,
              float gliss, float phi, float beta, float alpha, float amin,
              float cutoff, float pan1, float pan2, float pan3)
{
  const float argv[FOF_NUMARGS] = {
      ampl, freq, gliss, phi, beta, alpha, amin, cutoff, pan1, pan2, pan3};
  return fof_queue_add(&(jfofs->shmem->q), time_us, argv);
}

static inline uint64_t current_frame(jfofs_t *jfofs)
{
  fof_queue_t *q = &(jfofs->shmem->q);
  uint64_t frame_stamp;
  uint64_t current_source_frame;
  uint64_t next_frame;

  next_frame = __atomic_load_n(&(q->next_frame), __ATOMIC_ACQUIRE);
  frame_stamp = __atomic_load_n(&(q->frame_stamp), __ATOMIC_ACQUIRE);

  if (jfofs_clock_uses_transport(jfofs->shmem->setup.clock_source))
  {
#if 0
    jack_position_t pos;

    jack_transport_query(jfofs->j_client, &pos);
    current_source_frame = pos.frame;
#else
    current_source_frame = jack_get_current_transport_frame(jfofs->j_client);
#endif
  }
  else
  {
    current_source_frame = jack_frame_time(jfofs->j_client);
  }

  if (current_source_frame <= frame_stamp)
    return next_frame;

  return next_frame + (current_source_frame - frame_stamp);
}

uint64_t jfofs_get_frame(jfofs_t *jfofs)
{
  return current_frame(jfofs);
}

uint64_t jfofs_get_queue_next_frame(jfofs_t *jfofs)
{
  return __atomic_load_n(&(jfofs->shmem->q.next_frame), __ATOMIC_ACQUIRE);
}

int jfofs_set_clock_source(jfofs_t *jfofs, jfofs_clock_source_t clock_source)
{
  int normalized_clock_source;

  if (clock_source == JFOFS_CLOCK_OFFLINE_FREE_RUNNING)
    return JFOFS_FAILURE;

  normalized_clock_source = jfofs_clock_source_normalize(clock_source);
  __atomic_store_n(&(jfofs->shmem->q.clock_source), normalized_clock_source,
                   __ATOMIC_RELEASE);
  jfofs->shmem->setup.clock_source = normalized_clock_source;
  return JFOFS_SUCCESS;
}

jfofs_clock_source_t jfofs_get_clock_source(jfofs_t *jfofs)
{
  int clock_source;

  clock_source = __atomic_load_n(&(jfofs->shmem->q.clock_source),
                                 __ATOMIC_ACQUIRE);
  return jfofs_clock_source_normalize(clock_source);
}

jfofs_time_t jfofs_get_time(jfofs_t *jfofs)
{
  return current_frame(jfofs) * 1000000UL / jfofs->shmem->q.sample_rate;
}

int jfofs_sample_rate(jfofs_t *jfofs)
{
  return jfofs->shmem->q.sample_rate;
}

int jfofs_buffer_size(jfofs_t *jfofs)
{
  return jfofs->shmem->q.buffer_size;
}

shmem_t *jfofs_get_shmem(jfofs_t *jfofs)
{
  return jfofs->shmem;
}

setup_t *jfofs_get_setup(jfofs_t *jfofs)
{
  return &(jfofs->shmem->setup);
}

int jfofs_get_reference_cnt(jfofs_t *jfofs)
{
  return jfofs->shmem->reference_cnt;
}

int jfofs_has_statistics(jfofs_t *jfofs)
{
#ifdef STATISTICS_ENABLE
  return jfofs->shmem->has_statistics;
#else
  (void)jfofs;
  return false;
#endif
}

void *jfofs_get_statistics(jfofs_t *jfofs)
{
#ifdef STATISTICS_ENABLE
  if (jfofs->shmem->has_statistics)
    return &(jfofs->shmem->statistics);
  else
  {
    fprintf(stderr,
            "jfofs_get_statistics: statistics is not enabled in server\n");
    return NULL;
  }
#else
  (void)jfofs;
  fprintf(stderr,
          "jfofs_get_statistics: statistics is not enabled in server\n");
  return NULL;
#endif
}
