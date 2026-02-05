#include <stdlib.h>
#include <string.h>

#include "jfofs_types.h"
#include "manager_dummy.h"
#include "manager.h"

void *manager_dummy_ipc_loop(void *argp);

manager_dummy *manager_dummy_new(int *status)
{
  manager_dummy *dmgr = NULL;

  *status = posix_memalign((void **)&dmgr, CACHE_LINE_SIZE, sizeof(manager_dummy));

  if (*status != 0 || dmgr == NULL)
  {
    return NULL;
  }

  dmgr->mgr = NULL;
  *status = posix_memalign((void **)&(dmgr->mgr), CACHE_LINE_SIZE, sizeof(manager));

  if (*status != 0 || dmgr->mgr == NULL)
  {
    free(dmgr);
    return NULL;
  }

  memset((char *)(dmgr->mgr), 0, sizeof(manager));
  *status = manager_setup_ipc(dmgr->mgr);
  if (*status != JFOFS_SUCCESS)
  {
    free(dmgr->mgr);
    free(dmgr);
    return NULL;
  }
  return dmgr;
}

void manager_dummy_free(manager_dummy *dmgr)
{
  free(dmgr);
}

int manager_dummy_get_count(manager_dummy *dmgr)
{
  shm_t *shm = dmgr->mgr->shm;
  int n;

  sem_wait(&shm->sem);
  n = dmgr->count;
  sem_post(&shm->sem);
  return n;
}
void manager_dummy_get_fof(manager_dummy *dmgr, int idx, fof *_fof)
{
  shm_t *shm = dmgr->mgr->shm;

  sem_wait(&shm->sem);
  memcpy((char *)_fof, (char *)(dmgr->fof), sizeof(fof));
  sem_post(&shm->sem);
}

pthread_t manager_dummy_start(manager_dummy *dmgr)
{
  dmgr->tid = 0;
  pthread_create(&(dmgr->tid), NULL, manager_dummy_ipc_loop, (void *)dmgr);
  return dmgr->tid;
}

void *manager_dummy_ipc_loop(void *argp)
{
  manager_dummy *dmgr = (manager_dummy *)argp;
  shm_t *shm = dmgr->mgr->shm;

  while (1)
  {
    sem_wait(&shm->sem1);
    memcpy((char *)&(dmgr->fof[dmgr->count]), (char *)&(shm->fof), sizeof(fof));
    if (++dmgr->count >= MDUMMY_MAX_FOFS)
      dmgr->count = 0;
    sem_post(&shm->sem2);
  }
}
