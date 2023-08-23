#ifndef __JFOFS_H__
#define __JFOFS_H__

#include <stdint.h>
#include "jfofs_types.h"
#include "shmem.h"
//#include "api.h"

typedef struct jfofs_s jfofs_t;

jfofs_t* jfofs_new(int* status, shmem_t* shmem);
void jfofs_free(jfofs_t* jfofs);
int jfofs_add(jfofs_t* jfofs, jfofs_time_t time_us, float* fof_argv);
jfofs_time_t jfofs_get_time(jfofs_t* jfofs);
int jfofs_sample_rate(jfofs_t* jfofs);
int jfofs_buffer_size(jfofs_t* jfofs);
int jfofs_has_statistics(jfofs_t* jfofs);
void* jfofs_get_statistics(jfofs_t* jfofs);
shmem_t* jfofs_get_shmem(jfofs_t* jfofs);
setup_t* jfofs_get_setup(jfofs_t* jfofs);
int jfofs_get_reference_cnt(jfofs_t* jfofs);

#endif
