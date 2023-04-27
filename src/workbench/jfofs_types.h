#ifndef __jfofs_types_h__
#define __jfofs_types_h__

#define JFOFS_MAX_CHANS 8

typedef enum jfofs_status_e jfofs_status;

enum jfofs_status_e
{
  JFOFS_SUCCESS = 0x00,
  JFOFS_FALIURE = 0x01,
  JFOFS_MEMORY = 0x02,
  JFOFS_PORT_ERROR = 0x03, /* can't register jack port */
  JFOFS_JACK_ERROR = 0x10000
};

typedef struct jfofs_controller_s jfofs_controller;
typedef struct fof_s fof;
//typedef struct sin_gen_s sin_gen;


#endif /* __jfofs_types_h__ */
