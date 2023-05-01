#ifndef __jfofs_types_h__
#define __jfofs_types_h__

#define TIME_1 1000000ULL

typedef uint64_t jfofs_time;
typedef struct fof_s fof;
typedef enum jfofs_status_e jfofs_status;

enum jfofs_status_e
{
  JFOFS_SUCCESS = 0x00,
  JFOFS_FALIURE = 0x01,
  JFOFS_MEMORY = 0x02,
  JFOFS_PORT_ERROR = 0x03, /* can't register jack port */
  JFOFS_JACK_ERROR = 0x10000
};



#endif /* __jfofs_types_h__ */
