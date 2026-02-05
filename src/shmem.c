#include <unistd.h>
#include <fcntl.h>    /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "shmem.h"
#include "fof_queue.h"
#include "jfofs_private.h"
#include "debug.h"

size_t shmem_layout(setup_t *setup, size_t *slots_off, size_t *fofs_off);

static shmem_t *g_shmem;

shmem_t *shmem_create(setup_t *setup, int *status)
{
  int fd;
  size_t slots_offset;
  size_t fofs_offset;
  size_t size;

  shm_unlink(SHMEM_NAME);
  fd = shm_open(SHMEM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

  if (fd < 0)
  {
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }

  size = shmem_layout(setup, &slots_offset, &fofs_offset);
  if (ftruncate(fd, (off_t)size) < 0)
  {
    close(fd);
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }

  g_shmem = (shmem_t *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);

  if (g_shmem == MAP_FAILED)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }

  DEFINE_FOF_LIMITS((char *)g_shmem + fofs_offset,
                    (char *)g_shmem + fofs_offset + setup->n_max_fofs * sizeof(fof_t));

  printf("shmem base (server): %p size: %zu\n", (void *)g_shmem, size);
  g_shmem->base = g_shmem;
  g_shmem->size = size;
  g_shmem->q.slot = (fof_t **)((char *)g_shmem + slots_offset);
  memcpy((char *)&(g_shmem->setup), (char *)setup, sizeof(setup_t));

  /* not linked yet, linking is done in fof_queue_init */
  g_shmem->q.free_fofs = (fof_t *)((char *)g_shmem + fofs_offset);
  g_shmem->reference_cnt = 0;
  g_shmem->has_statistics = HAS_STATISTICS;
  STATISTICS_INIT;
  return g_shmem;
}

void *shmem_get_fof_slots_ptr(shmem_t *shmem, setup_t *setup)
{
  (void)setup;
  return (void *)((char *)shmem + sizeof(shmem_t));
}

void *shmem_get_fof_mem_ptr(shmem_t *shmem, setup_t *setup)
{
  return (void *)((char *)shmem + sizeof(shmem_t) +
                  sizeof(void *) * setup->n_slots);
}

shmem_t *shmem_link(int *status)
{
  int fd;
  void *base;
  size_t size;
  size_t slots_offset;
  size_t fofs_offset;

  if ((fd = shm_open(SHMEM_NAME, O_RDWR, 0)) == -1)
  {
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }

  g_shmem = (shmem_t *)mmap(NULL, sizeof(shmem_t), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);

  if (g_shmem == MAP_FAILED)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }

  base = (void *)g_shmem->base;
  size = g_shmem->size;
  printf("shmem base (client): %p size: %zu\n", base, size);
  munmap((void *)g_shmem, sizeof(shmem_t));
  printf("shmem mapped at first to: %p\n", (void *)g_shmem);
  /* mapping to make pointers work or mapping fails */
  g_shmem = (shmem_t *)mmap(base, size, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_FIXED, fd, 0);
  close(fd);

  size = shmem_layout(&(g_shmem->setup), &slots_offset, &fofs_offset);
  DEFINE_FOF_LIMITS((char *)g_shmem + fofs_offset,
                    (char *)g_shmem + fofs_offset + g_shmem->setup.n_max_fofs * sizeof(fof_t));

  printf("shmem mapped to: %p\n", (void *)g_shmem);
  if (g_shmem == MAP_FAILED || g_shmem != base)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }

  g_shmem->reference_cnt++;
  return g_shmem;
}

shmem_t *shmem_ptr(void)
{
  return g_shmem;
}

void shmem_unmap(shmem_t *shm)
{
  size_t size = shm->size;

  shm->reference_cnt--;
  if (g_shmem == shm)
    g_shmem = NULL;

  munmap((void *)shm, size);
}

void shmem_unlink(shmem_t *shm)
{
  (void)shm;
  shm_unlink(SHMEM_NAME);
}

char *shmem_aligning_ptr(char *ptr, size_t alignment_size)
{
  size_t rest = (size_t)ptr % alignment_size;

  if (rest == 0)
    return ptr;
  else
    return ptr + alignment_size - rest;
}

size_t shmem_layout(setup_t *setup, size_t *slots_off, size_t *fofs_off)
{
  *slots_off = (size_t)shmem_aligning_ptr((char *)sizeof(shmem_t),
                                          CACHE_LINE_SIZE);
  *fofs_off = *slots_off + sizeof(fof_t *) * setup->n_slots;
  *fofs_off = (size_t)shmem_aligning_ptr((char *)*fofs_off, CACHE_LINE_SIZE);
  return *fofs_off + sizeof(fof_t) * setup->n_max_fofs;
}
