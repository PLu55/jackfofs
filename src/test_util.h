#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include <stdio.h>
#include <math.h>

#include "jfofs.h"
#include "jfofs_private.h"

void fof_default(fof_t* fof);
void fof_print(fof_t* fof);
int fof_equal(fof_t* fof1, fof_t* fof2);

void dump_setup(jfofs_t* jfofs);
void jfofs_sleep(jfofs_time_t t);

#endif
