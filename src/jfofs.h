#ifndef __JFOFS_H__
#define __JFOFS_H__

typedef struct jfofs_s jfofs;
typedef struct fof_s fof;

jfofs* jfofs_new(int* status);
void jfofs_free(jfofs* _jfofs);
void jfofs_add(jfofs* _jfofs, fof* _fof);
  
#endif
