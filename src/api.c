#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

#include "jfofs.h"
#include "jfof_private.h"
#include "ipc.h"

struct jfofs_s
{
  shm_t* shm;
  int fd;
}
  
jfofs* jfofs_new()
{
  jfofs* _jfofs;
    char shm_name = SHM_NAME;
  size_t shm_size = sizeof(shm_t);
  
  *status = posix_memalign((void**) &_jfofs, CACHE_LINE_SIZE, sizeof(jfofs));
  if (dsp == NULL)
  {
    return NULL;
  }
  _jfofs->fd = shm_open(&shm_name, O_RDWR, 0);
  if (_jfofs->fd < 0)
    error;
  _jfofs->shm = (shm_t*) mmap(NULL, shm_size , PROT_READ | PROT_WRITE, MAP_SHARED,
			      _jfofs->fd, 0);
  return _jfofs;
}

void jfofs_free(jfofs* _jfofs)
{
  close(_jfofs->fd);
  free(_jfofs);
}

void jfofs_add(jfofs* _jfofs, fof* _fof)
{
  sem_wait(&_fofs->shm->sem);
  memcpy(&_fofs->shm->fof, _fof, sizeof(fof))
  sem_post(&_fofs->shm->sem);
}
