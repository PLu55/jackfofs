#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include <math.h>

#include "jfofs_private.h"

void fof_default(fof* _fof);

inline double fof_alpha_to_t60(double alpha)
{
  return log(1e-6) / -alpha;
}

inline double fof_t60_to_alpha(double t60)
{
  return log(1e-6) / -t60;
}

inline double fof_alpha_to_bw(double alpha)
{
  return alpha * M_PI;
}

inline double fof_bw_to_alpha(double bw)
{
  return bw / M_PI;
}

#endif
