#ifndef __JFOFS_PRIVATE_H__
#define __JFOFS_PRIVATE_H__

#include <fofs.h>

#define CACHE_LINE_SIZE 64
#define JFOFS_MAX_CHANS 8

typedef struct ctrl_client_s ctrl_client;
typedef struct dsp_client_s dsp_client;
typedef struct mix_client_s mix_client;
typedef struct fof_queue_s fof_queue;
typedef struct chunk_s chunk;
typedef struct fof_s fof;

struct fof_s
{
  double time;
  float argv[FOF_NUMARGS];
};

#endif /* __jfofs_types_h__ */
