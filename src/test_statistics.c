#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>

#include "statistics.h"
#include "shmem.h"
#include "test_util.h"

void test_statistics(void)
{
  int status;
  setup_t setup;
  shmem_t *shmem;

  setup.clock_source = JFOFS_CLOCK_JACK_FRAME_TIME;
  setup.mode = FOF_MONO;
  setup.n_clients = 1;
  setup.n_preallocate_fofs = 128;
  setup.n_max_fofs = 128;
  setup.n_slots = 8;
  setup.sample_rate = 48000;
  setup.max_buffer_size = 256;

  shmem = shmem_create(&setup, &status);
  TEST_ASSERT_NOT_NULL(shmem);

#ifdef STATISTICS_ENABLE
  statistics_init();
  statistics_t *stats = &(shmem->statistics);

  TEST_ASSERT_EQUAL_INT(setup.n_slots, stats->n_slots);
  for (int i = 0; i < stats->n_slots; i++)
    TEST_ASSERT_EQUAL_INT(0, stats->slot_cnt[i]);
  TEST_ASSERT_EQUAL_INT(0, stats->late_cnt);
  TEST_ASSERT_EQUAL_INT(0, stats->excess_cnt);

  /* increment some slot counters */
  incr_slot_cnt(0);
  incr_slot_cnt(0);
  incr_slot_cnt(3);
  TEST_ASSERT_EQUAL_INT(2, stats->slot_cnt[0]);
  TEST_ASSERT_EQUAL_INT(1, stats->slot_cnt[3]);

  /* increment late and excess */
  incr_late_cnt();
  incr_excess_cnt();
  TEST_ASSERT_EQUAL_INT(1, stats->late_cnt);
  TEST_ASSERT_EQUAL_INT(1, stats->excess_cnt);

  /* mark first N free fofs as "used" by setting time_us != 0 */
  fof_t *f = shmem->q.free_fofs;
  const int k = 5;
  for (int i = 0; i < k && f; i++, f = f->next)
    f->time_us = 1;

  /* compute expected prefix count of used free fofs */
  int expected = 0;
  fof_t *g = shmem->q.free_fofs;
  while (g && g->time_us != 0)
  {
    expected++;
    g = g->next;
  }
  TEST_ASSERT_EQUAL_INT(expected, max_used_free_list());

  /* ensure dump doesn't crash (prints to stdout) */
  dump_statistics();
#else
  /* If statistics are disabled, do nothing (test is a no-op). */
#endif

  /* leave shmem mapped; tests in this suite don't unmap explicitly */
}
