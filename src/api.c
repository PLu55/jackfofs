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

struct jfofs_s
{
  shm_t* shm;
  int fd;
};
  
jfofs* jfofs_new(int* status)
{
  jfofs* _jfofs;
  size_t shm_size = sizeof(shm_t);
  
  *status = posix_memalign((void**) &_jfofs, CACHE_LINE_SIZE, sizeof(jfofs));
  if (_jfofs == NULL)
  {
    *status = JFOFS_MEMORY;
    return NULL;
  }
  _jfofs->fd = shm_open(SHMEM_NAME, O_RDWR, 0);

  if (_jfofs->fd < 0)
  {
    free(_jfofs);
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }
  
  _jfofs->shm = (shm_t*) mmap(NULL, sizeof(shm_t) , PROT_READ | PROT_WRITE,
			      MAP_SHARED, _jfofs->fd, 0);
  if (_jfofs->shm == NULL)
  {
    jfofs_free(_jfofs);
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }

  return _jfofs;
}

void jfofs_free(jfofs* _jfofs)
{
  close(_jfofs->fd);
  free(_jfofs);
}

void jfofs_add(jfofs* _jfofs, fof* _fof)
{
  sem_wait(&_jfofs->shm->sem2);
  memcpy(&_jfofs->shm->fof, _fof, sizeof(fof));
  sem_post(&_jfofs->shm->sem1);
}
