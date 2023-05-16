#ifndef __JFOFS_H__
#define __JFOFS_H__

#include <stdint.h>

typedef struct jfofs_s jfofs_t;

jfofs_t* jfofs_new(int* status);
void jfofs_free(jfofs_t* jfofs);
int jfofs_add(jfofs_t* jfofs, uint64_t time_us, float* fof_argv);
  
#endif
