#ifndef __MANAGER_DUMMY_H__
#define __MANAGER_DUMMY_H__

#include <stdlib.h>

#include "manager.h"

typedef struct manager_dummy_s manager_dummy;

#define MDUMMY_MAX_FOFS 64

struct manager_dummy_s
{
  manager* mgr;
  pthread_t tid;
  int count;
  fof fof[MDUMMY_MAX_FOFS];
};

manager_dummy* manager_dummy_new(int* status);
void manager_dummy_free(manager_dummy* dmgr);
int manager_dummy_get_count(manager_dummy* dmgr);
void manager_dummy_get_fof(manager_dummy* dmgr, int idx, fof* _fof);
pthread_t manager_dummy_start(manager_dummy* dmgr);

#endif

