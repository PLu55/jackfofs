#include <jack/jack.h>

#include "process_queue.h"
#include "controller.h"
#include "jfof.h"
#include "jfof_private.h"

fof_queue* fof_queue_new(int sample_rate, int n_slots, int slot_size,
			 int n_free_chunks, int chunk_size, int* status);

jfofs_controller* jfofs_controller_new(FofsMode mode, int nclients,
				       int nchans, int* status);
typedef struct manager_s manager
struct manager_s
{
};
  
void manager(void)
{
  
  
}

inline double systime()
{
  struct timespec tspec;

  clock_gettime(CLOCK_REALTIME, &tspec);
  return (double)tspec.tv_sec + (double)tspec.tv_nsec*1.0e-9;
}

struct PLu_Fofs : public Unit
{
  FofBank* mBank;
  int mState;
  double mDtma;
  double mTcorr;
  double mTframe;
#ifdef DEBUG
  long frameCnt;
#endif
};

/* At each buffer cycle in jack */
void time_handling()
{
  double alpha = 0.01;
  double tf, ts, dt;
  
  tf = fof_time(unit->mBank); /* frame counting */
  ts = systime();
  unit->mTframe = ts;
  dt = ts - tf;
  unit->mDtma = alpha * dt + (1.0-alpha) * unit->mDtma ;

  if( fabs(unit->mDtma - dt) > 1.0)
    unit->mDtma = dt;
        
    unit->mTcorr = round(unit->mDtma*1e4)*1e-4;
}
