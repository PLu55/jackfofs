#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include "jfofs_private.h"
#include "config.h"

#ifdef STATISTICS_ENABLE

struct statistics_s
{
  int excess_cnt;
  int late_cnt;
  int n_slots;
  int slot_cnt[MAX_SLOTS];
};

typedef struct statistics_s statistics_t;

void statistics_init();
void incr_slot_cnt(int slot);
void incr_late_cnt();
void incr_excess_cnt();
void dump_statistics(void);

#define STATISTICS_T(var) statistics_t var
#define STATISTICS_FIELD(var) statistics_t var;
#define HAS_STATISTICS 1
#define STATISTICS_INIT statistics_init()
#define INCR_SLOT_CNT(slot) incr_slot_cnt(slot)
#define INCR_LATE_CNT() incr_late_cnt()
#define INCR_EXCESS_CNT() incr_excess_cnt()
#define DUMP_STATISTICS() dump_statistics()

#else

#define STATISTICS_T(var)
#define STATISTICS_FIELD(var)
#define HAS_STATISTICS 0
#define STATISTICS_INIT
#define INCR_SLOT_CNT(slot)
#define INCR_LATE_CNT()
#define INCR_EXCESS_CNT()
#define DUMP_STATISTICS()

#endif

#endif
