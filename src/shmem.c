#include <unistd.h>
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>

#include "shmem.h"
#include "fof_queue.h"
#include "jfofs_private.h"

#define MEMORY_ALIGNMENT 8UL

size_t shmem_layout(setup_t* setup, size_t* slots_off, size_t* fofs_off);

shmem_t* shmem_create(setup_t* setup, int* status)
{
  int fd;
  shmem_t* shmem;
  size_t slots_offset;
  size_t fofs_offset;
  size_t size;
    
  shm_unlink(SHMEM_NAME);
  fd = shm_open(SHMEM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR|S_IWUSR);
  
  if (fd < 0)
  {
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }
  
  size = shmem_layout(setup, &slots_offset, &fofs_offset);
  ftruncate(fd, size);
 
  shmem = (shmem_t*) mmap(NULL, size , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shmem == NULL)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }
  
  shmem->base = shmem;
  shmem->q.slot = (fof_t**)((char*)shmem + slots_offset);
  
  /* not linked yet, linking is done in fof_queue_init */
  shmem->q.free_fofs =  (fof_t*)((char*)shmem + fofs_offset);
				 
  return shmem;
}

void* shmem_get_fof_slots_ptr(shmem_t* shmem, setup_t* setup)
{
  return (void*)((char*)shmem + sizeof(shmem_t)); 
}

void* shmem_get_fof_mem_ptr(shmem_t* shmem, setup_t* setup)
{
  return (void*)((char*)shmem + sizeof(shmem_t) +
		 sizeof(void*) * setup->n_slots);
}

shmem_t* shmem_link(setup_t* setup, int* status)
{
  int fd;
  shmem_t* shmem;
  size_t size;
  size_t slots_off;
  size_t fofs_off;
  
  fd = shm_open(SHMEM_NAME, O_RDWR, 0);
  size = shmem_layout(setup, &slots_off, &fofs_off);
  
  shmem = (shmem_t*) mmap(NULL, size , PROT_READ | PROT_WRITE,
			  MAP_SHARED, fd, 0);

  if (shmem == NULL)
  {
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }
  
  if (shmem != shmem->base)
  {
    void* base = (void*)shmem->base;
    
    munmap((void*) base, size);

    shmem = (shmem_t*) mmap(base, size, PROT_READ | PROT_WRITE | MAP_FIXED,
			MAP_SHARED, fd, 0);
    if (shmem == NULL)
    {
      *status = JFOFS_SHM_MAP_ERROR;
      return NULL;
    }
  }
  return shmem;
}

char* shmem_aligning_ptr(char* ptr, size_t alignment_size)
{
  size_t rest = (size_t)ptr % alignment_size;

  if (rest == 0)
    return ptr;
  else
    return ptr + alignment_size - rest;
}

size_t shmem_layout(setup_t* setup, size_t* slots_off, size_t* fofs_off)
{
  *slots_off = (size_t)shmem_aligning_ptr((char*) sizeof(shmem_t),
					  MEMORY_ALIGNMENT);
  *fofs_off = *slots_off + sizeof(fof_t*) * setup->n_slots;
  *fofs_off = (size_t)shmem_aligning_ptr((char*) *fofs_off, MEMORY_ALIGNMENT);
  return *fofs_off + sizeof(fof_t) * setup->n_max_fofs;
}
