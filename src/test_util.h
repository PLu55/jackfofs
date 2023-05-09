#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include <stdio.h>
#include <math.h>

#include "jfofs_private.h"

void fof_default(fof* _fof);
void fof_print(fof* f);
int fof_equal(fof* fof1, fof* fof2);

#endif
