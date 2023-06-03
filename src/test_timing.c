#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <jack/jack.h>

#include "jfofs.h"

/*
 * t1 is clock_gettime CLOCK_MONOTONIC_RAW
 * t2 is clock_gettime CLOCK_REALTIME
 * t3 is jfofs_get_time
 * t4 is jack_get_time
 * t5 is jack_frame_time * 1000000 / fs
 *
 * t2 - t1 is drifting about 3.3ppm
 * t5 - t3 is not drifting jitter is about 20us
 * t5 - t4 is drifting about 3.3ppm
 */

/*
 * maverage computes the moving average of length W where x is the
 * value to take the average on, alpha is 1/W, mu is the last mu
 */

double maverage(double x, double mu, double alpha)
{  
  return (1.0 - alpha) * mu + alpha * x; 
}

static inline uint64_t now(clockid_t cid)
{
  struct timespec tp;
  clock_gettime(cid, &tp);
  return tp.tv_sec * 1000000UL + tp.tv_nsec / 1000UL;
}

static inline uint64_t jack_time_from_frames(jack_client_t* client, int sample_rate)
{
  return jack_frame_time(client) * 1000000UL / sample_rate;
}

double mu = 0.0;
double mu_old = 0.0;
double dmu = 0.0;

int64_t d_old = 0;
int64_t d_old2 = 0;
int64_t diff = 0;

void compare_clocks(uint64_t t1, uint64_t t2, double alpha)
{
  //*old = diff;
  diff = ((int64_t) t2 - (int64_t) t1);

  mu = maverage((double) diff, mu, alpha);
  //dmu = maverage(diff-old2, dmu, alpha);
  printf("t1: %ld\n", t1);
  //printf("t2: %ld diff:%ld mu: %f dmu: %f\n", t2, diff, mu, dmu);
  printf("t2: %ld diff:%ld mu: %f\n", t2, diff, mu);
}

int main (int argc, char **argv)
{
  int status;
  struct timespec res;
  struct timespec ts;
  struct timespec rem;
  //time_t tv_sec;	       
  //long tv_nsec;
  uint64_t t1_0, t2_0, t3_0, t4_0, t5_0;
  uint64_t t1, t2, t3, t4, t5, tdiff, tdl;
  double mu;
  double t4_mu = 0.0;
  uint64_t sample_rate;
  
  jfofs_t* jfofs = jfofs_new(&status);  

  if (jfofs == NULL)
  {
    if (status == JFOFS_SHM_ERROR)
      fprintf(stderr, "JFofs Error: jfofs could not be created, server not running.\n");
    else
      fprintf(stderr, "JFofs Error: jfofs shared memory could not be mapped.\n");
    exit(1);
  }
  sample_rate = jfofs_sample_rate(jfofs);
  jack_client_t* jc = jack_client_open("jfofs_timing", 0, NULL); 
  jack_activate(jc);

  clock_getres(CLOCK_MONOTONIC_RAW, &res);
  printf("Resolution MONOTONIC_RAW: %ld %ld\n",  res.tv_sec, res.tv_nsec);
  clock_getres(CLOCK_REALTIME, &res);
  printf("Resolution REALTIME: %ld %ld\n",  res.tv_sec, res.tv_nsec);

  ts.tv_sec = 1L;
  ts.tv_nsec = 0L;
  t1_0 = now(CLOCK_MONOTONIC_RAW);
  t2_0 = now(CLOCK_REALTIME);
  printf("A\n");
  t3_0 = jfofs_get_time(jfofs);
  printf("B\n");
  t4_0 = jack_get_time();

  t5_0 = jack_time_from_frames(jc, 48000);

  double diff = (double)( (int64_t)t5_0 - (int64_t)t4_0);
  int i = 0;
  for (;;)
  {
    printf("Trial: %d\n", ++i);
    
    t1 = now(CLOCK_MONOTONIC_RAW) - t1_0;
    t4 = jack_get_time() - t4_0;
    t2 = now(CLOCK_REALTIME) - t2_0;
    t3 = jfofs_get_time(jfofs) - t3_0;
    t5 = jack_time_from_frames(jc, 48000) - t5_0;

    compare_clocks(t5, t3, 1.0/100.0);
    
#if 0
    printf("t1: %ld\n", t1);
    printf("t2: %ld %ld\n", t2, t2 - t1);
    printf("t3: %ld %ld\n", t3, t3 - t1);
    diff = (double)((int64_t)t5 - (int64_t)t4 );
    printf("diff %f\n", diff);
    t4_mu = maverage(diff, t4_mu, 1.0/10);
    printf("t4: %f\n" , t4_mu);
    printf("t4: %ld %ld %f\n", t4, t5 - t4 , t4_mu);
    printf("t5: %ld %ld\n", t5, t5 - t1);
    tdiff = t5-t1;
    printf("tdiff: %ld tdd: %ld\n", tdiff, tdiff - tdl);
    tdl = tdiff;
#endif
    nanosleep(&ts, &rem);
  }
}



/*
  CLOCK_REALTIME
  CLOCK_REALTIME_ALARM
  CLOCK_REALTIME_COARSE
  CLOCK_TAI
  CLOCK_MONOTONIC
  CLOCK_MONOTONIC_COARSE
  CLOCK_MONOTONIC_RAW     <--------
  CLOCK_BOOTTIME
  CLOCK_BOOTTIME_ALARM
  CLOCK_PROCESS_CPUTIME_ID
  CLOCK_THREAD_CPUTIME_ID 
*/
