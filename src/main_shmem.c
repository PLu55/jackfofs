#include <unistd.h>
#include <stdio.h>
#include <unity/unity.h>

#include "shmem.h"

int main(int argc, char** argv)
{
  shmem_t* shmem;

  pid_t pid = getpid();

  fork();
    
  if (getpid() == pid)
  {
    shmem = shmem_create();
    printf("Master maps to: %p\n", shmem);
    sleep(-1);
  }
  else
  {
    shmem = shmem_link();
    printf("Client maps to: %p\n", shmem);
    sleep(-1);
  }
  
}
