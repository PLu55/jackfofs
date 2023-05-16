#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "jfofs.h"
#include "jfofs_types.h"
#include "jfofs_private.h"
#include "shmem.h"

struct jfofs_s
{
  shm_t* shm;
  int fd;
};
  
jfofs_t* jfofs_new(int* status)
{
  jfofs_t* jfofs;
  size_t shm_size = sizeof(shm_t);
  
  *status = posix_memalign((void**) &jfofs, CACHE_LINE_SIZE, sizeof(jfofs));
  if (jfofs == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }

  jfofs->shmem = shmem_link(status);

  if (jfofs->shmem == NULL)
  {
    free(jfofs);
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }

  return jfofs;
}

void jfofs_free(jfofs_t* jfofs)
{
  shmem_unlink(jfofs->shmem)
  free(jfofs);
}

int jfofs_add(jfofs_t* jfofs, uint64_t time_us, float* fof_argv)
{
  return fof_queue_add(jfofs->shmem-q, time_us, fof_argv);
}
