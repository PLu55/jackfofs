#include <stdio.h> // kan tas bort
#include <stdbool.h>
#include <fofs.h>

#include "jfofs_private.h"
#include "jfofs_types.h"

struct fof_in_s
{
  uint64_t time_us; /* time in microsecunds */
  float argv[FOF_NUMARGS];
};

struct shmem_s
{
  shmem_t *base;
  size_t size;
  setup_t setup;
  uint32_t count;
};

/* It will maybe be wrong when cnt is zero */

/* works only with one single client */
int add_fof(shmem_t *shmem, fof_in_s *fof)
{
  uint32_t cnt;
  for (;;)
  {
    cnt = __atomic_load_n(shmem->count, _ATOMIC_ACQUIRE);
    if (cnt == UINT32_MAX)
      cnt = 0;
    memcpy((char *)shmem + (cnt + 1) * sizeof(fof), fof, sizeof(fof));
    if (__atomic_compare_exchange_n(&shmem->count, cnt, cnt + 1, __ATOMIC_RELEASE))
      break;
  }
}

int get_fofs(shmem_t *shmem, fof_queue_t *q)
{
  uint32_t cnt;
  fof_t *fof;
  fof_in_s *fof_in;

  cnt = __atomic_exchange_n(&shmem->count, UINT32_MAX, _ATOMIC_ACQUIRE);
  if (cnt == UINT32_MAX)
    return 0;
  for (uint32_t i = 0; i < cnt; i++)
  {
    fof_in = (char *)shmem + i * sizeof(fof_in_t);
    fof_queue_add(q, fof_in->time_us, &(fof_in->argv));
  }
  return cnt;
}
