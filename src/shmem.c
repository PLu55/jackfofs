#include <unistd.h>
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "shmem.h"
#include "fof_queue.h"
#include "jfofs_private.h"
#include "debug.h"

size_t shmem_layout(setup_t* setup, size_t* slots_off, size_t* fofs_off);

shmem_t* shmem;

shmem_t* shmem_create(setup_t* setup, int* status)
{
  int fd;
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
  close(fd);
  
  if (shmem == MAP_FAILED)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }
  
  DEFINE_FOF_LIMITS((char*)shmem + fofs_offset,
		    (char*)shmem+fofs_offset + setup->n_max_fofs * sizeof(fof_t));
  
  printf("shmem base (server): %p size: %ld\n", shmem, size);
  shmem->base = shmem;
  shmem->size = size;
  shmem->q.slot = (fof_t**)((char*)shmem + slots_offset);
  memcpy ((char*)&(shmem->setup), (char*)setup, sizeof(setup_t));
  
  /* not linked yet, linking is done in fof_queue_init */
  shmem->q.free_fofs =  (fof_t*)((char*)shmem + fofs_offset);

  shmem->has_statistics = HAS_STATISTICS; 
  STATISTICS_INIT;		 
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

shmem_t* shmem_link(int* status)
{
  int fd;
  void* base;
  size_t size;
  size_t slots_offset;
  size_t fofs_offset;
  
  if ((fd = shm_open(SHMEM_NAME, O_RDWR, 0)) == -1)
  {  
    *status = JFOFS_SHM_ERROR;
    return NULL;
  }
  
  shmem = (shmem_t*) mmap(NULL, sizeof(shmem_t) , PROT_READ | PROT_WRITE,
			  MAP_SHARED, fd, 0);

  if (shmem == MAP_FAILED)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }
  
  base = (void*)shmem->base;
  size = shmem->size;
  printf("shmem base (client): %p size: %ld\n", base, size);
  munmap((void*) shmem, sizeof(shmem_t));
  printf("shmem mapped at first to: %p\n", shmem); 
  /* mapping to make pointers work or mapping fails */
  shmem = (shmem_t*) mmap(base, size, PROT_READ | PROT_WRITE | MAP_FIXED,
  			MAP_SHARED, fd, 0);
  close(fd);

  size = shmem_layout(&(shmem->setup), &slots_offset, &fofs_offset);
  DEFINE_FOF_LIMITS((char*)shmem + fofs_offset,
  	    (char*)shmem+fofs_offset + shmem->setup.n_max_fofs * sizeof(fof_t));
  
  printf("shmem mapped to: %p\n", shmem); 
  if (shmem == MAP_FAILED || shmem != base)
  {
    *status = JFOFS_SHM_MAP_ERROR;
    return NULL;
  }

  return shmem;
}

shmem_t* shmem_ptr(void)
{
  return shmem;
}

void shmem_unmap(shmem_t* shmem)
{
  size_t size = shmem->size;
  munmap((void*) shmem, size);
}

void shmem_unlink(shmem_t* shmem)
{
  shm_unlink(SHMEM_NAME);
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
					  CACHE_LINE_SIZE);
  *fofs_off = *slots_off + sizeof(fof_t*) * setup->n_slots;
  *fofs_off = (size_t)shmem_aligning_ptr((char*) *fofs_off, CACHE_LINE_SIZE);
  return *fofs_off + sizeof(fof_t) * setup->n_max_fofs;
}
