#ifndef __JFOFS_H__
#define __JFOFS_H__

typedef struct jfofs_s jfofs_t;
typedef struct fof_s fof_t;

jfofs_t* jfofs_new(int* status);
void jfofs_free(jfofs_t* jfofs);
void jfofs_add(jfofs_t* jfofs, fof_t* fof);
  
#endif
