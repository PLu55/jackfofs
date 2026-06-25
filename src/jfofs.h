#ifndef __JFOFS_H__
#define __JFOFS_H__

#include <stdint.h>
#include "jfofs_types.h"
#include "shmem.h"
// #include "api.h"

typedef struct jfofs_s jfofs_t;

const char *jfofs_version(void);
jfofs_t *jfofs_new(int *status, shmem_t *shmem);
void jfofs_free(jfofs_t *jfofs);
int jfofs_add(jfofs_t *jfofs, uint64_t time_us, float ampl, float freq,
              float gliss, float phi, float beta, float alpha, float amin,
              float cutoff, float pan1, float pan2, float pan3);
jfofs_clock_source_t jfofs_get_clock_source(jfofs_t *jfofs);
int jfofs_set_clock_source(jfofs_t *jfofs, jfofs_clock_source_t clock_source);
uint64_t jfofs_get_frame(jfofs_t *jfofs);
/* Raw q->next_frame — use to detect when the ctrl_client has synced to a new
   transport position after jack_transport_locate().  Unlike jfofs_get_frame,
   this returns the uninterpolated queue counter, so it only reaches the new
   transport position after the ctrl_client JACK callback fires (~1 period). */
uint64_t jfofs_get_queue_next_frame(jfofs_t *jfofs);
jfofs_time_t jfofs_get_time(jfofs_t *jfofs);
int jfofs_sample_rate(jfofs_t *jfofs);
int jfofs_buffer_size(jfofs_t *jfofs);
int jfofs_has_statistics(jfofs_t *jfofs);
void *jfofs_get_statistics(jfofs_t *jfofs);
shmem_t *jfofs_get_shmem(jfofs_t *jfofs);
setup_t *jfofs_get_setup(jfofs_t *jfofs);
int jfofs_get_reference_cnt(jfofs_t *jfofs);

#endif
