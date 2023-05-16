#ifndef __jfofs_types_h__
#define __jfofs_types_h__

#include <stdint.h>

#define TIME_1 1000000ULL

typedef uint64_t jfofs_time_t;
typedef enum jfofs_status_e jfofs_status_t;

enum jfofs_status_e
{
  JFOFS_SUCCESS = 0x00,
  JFOFS_FALIURE = 0x01,
  JFOFS_MEMORY_ERROR = 0x02,       /* can't allocate memory */
  JFOFS_PORT_ERROR = 0x03,         /* can't register jack port */
  JFOFS_SHM_ERROR = 0x04,          /* can't open shared memory */
  JFOFS_SHM_MAP_ERROR = 0x05,      /* can't map shared memory */
  JFOFS_FOF_LIMIT_ERROR = 0x06,    /* num of fof limit is exceeded */
  JFOFS_FOF_LATE_WARNING  = 0x07,    /* fof is to late, it's ignored. */
  JFOFS_FOF_EXCESS_INFO  = 0x1007, /* fof exceeds the fof queue. */
  JFOFS_JACK_ERROR_MASK = 0x10000,
};

#endif /* __jfofs_types_h__ */
