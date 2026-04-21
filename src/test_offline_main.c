#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

#include "unity/unity.h"

void test_offline_create_close(void);
void test_offline_invalid_path(void);
void test_offline_mono_renders(void);
void test_offline_stereo_renders(void);
void test_offline_multiple_fofs(void);

void setUp(void) {}
void tearDown(void) {}

static void timeout_handler(int signum)
{
  (void)signum;
  fprintf(stderr, "jfofs_offline_test: timeout reached, aborting.\n");
  _exit(124);
}

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  signal(SIGALRM, timeout_handler);
  alarm(30);

  UNITY_BEGIN();
  RUN_TEST(test_offline_create_close);
  RUN_TEST(test_offline_invalid_path);
  RUN_TEST(test_offline_mono_renders);
  RUN_TEST(test_offline_stereo_renders);
  RUN_TEST(test_offline_multiple_fofs);
  int rc = UNITY_END();

  alarm(0);
  return rc;
}
