#ifndef __JFOFS_H__
#define __JFOFS_H__

static inline jack_nframes_t jfofs_time_us_to_nframe(uint64_t t, int sample_rate)
{
  return t * sample_rate / 1000000ULL;
}

static inline uint64_t jfofs_nframes_to_time_us(uint64_t n, int sample_rate)
{
  return n * 1000000ULL / sample_rate;
}

#endif
