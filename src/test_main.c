#include <unistd.h>

#include "unity/unity.h"

void test_fof_queue_init(void);
void test_fof_queue_add(void);
void test_fof_queue_free_list(void);
void test_dsp_client(void);
void test_ctrl_client(void);
void test_manager(void);
void test_mix_client(void);
void test_api(void);
void test_shmem(void);

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

int main (int argc, char **argv)
{
  UNITY_BEGIN();
  //RUN_TEST(test_fof_queue_init);
  //RUN_TEST(test_fof_queue_add);
  //RUN_TEST(test_fof_queue_free_list);
  //RUN_TEST(test_dsp_client);
  //RUN_TEST(test_ctrl_client);
  //RUN_TEST(test_mix_client);
  //RUN_TEST(test_manager);
  //RUN_TEST(test_api);
  RUN_TEST(test_shmem);

  return UNITY_END();
}
