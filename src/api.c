#include <fcntl.h>    /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "jfofs.h"
#include "jfofs_types.h"
#include "jfofs_private.h"
#include "api.h"
#include "shmem.h"
#include "statistics.h"
#include "test_util.h"

/* TODO: Implement a check if statistics is included in shmem or not */

jfofs_t* jfofs_new(int* status, shmem_t* shmem)
{
  jfofs_t* jfofs;
  jack_status_t jstatus;

  *status = posix_memalign((void**) &jfofs, CACHE_LINE_SIZE, sizeof(jfofs_t));
  if (jfofs == NULL)
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

  if( shmem != NULL)
  {
    jfofs->shmem = shmem;
  }
  else
  {
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

void jfofs_free(jfofs_t* jfofs)
{
  jack_deactivate(jfofs->j_client);
  jack_client_close(jfofs->j_client);
  shmem_unmap(jfofs->shmem);
  free(jfofs);
}

int jfofs_add(jfofs_t* jfofs, uint64_t time_us, float* fof_argv)
{
  return fof_queue_add(&(jfofs->shmem->q), time_us, fof_argv);
}

static inline uint64_t current_frame(jfofs_t* jfofs)
{
  uint64_t m;
  uint64_t n;

  n = jfofs->shmem->q.next_frame;
  m = jack_frame_time(jfofs->j_client) - jfofs->shmem->q.frame_stamp;
  return  (n + m);
}

uint64_t jfofs_get_frame(jfofs_t* jfofs)
{
  return current_frame(jfofs);
}

jfofs_time_t jfofs_get_time(jfofs_t* jfofs)
{
  return  current_frame(jfofs) * 1000000UL / jfofs->shmem->q.sample_rate;
}

int jfofs_sample_rate(jfofs_t* jfofs)
{
  return jfofs->shmem->q.sample_rate;
}

shmem_t* jfofs_get_shmem(jfofs_t* jfofs)
{
  return &(jfofs->shmem);
}

setup_t* jfofs_get_setup(jfofs_t* jfofs)
{
  return &(jfofs->shmem->setup);
}

jfofs_get_reference_cnt(jfofs_t* jfofs)
{
  return jfofs->shmem->reference_cnt;
}

int jfofs_has_statistics(jfofs_t* jfofs)
{
#ifdef STATISTICS_ENABLE
  return jfofs->shmem->has_statistics;
#else
  return false;
#endif
}

void* jfofs_get_statistics(jfofs_t* jfofs)
{
#ifdef STATISTICS_ENABLE
  if (jfofs->shmem->has_statistics)
    return &(jfofs->shmem->statistics);
  else {
    fprintf(stderr,
	    "jfofs_get_statistics: statistics is not enabled in server\n");
    return NULL;
  }
#else
  fprintf(stderr,
	  "jfofs_get_statistics: statistics is not enabled in server\n");
  return NULL;
#endif
}
