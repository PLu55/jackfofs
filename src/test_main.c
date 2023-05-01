#include <unistd.h>

#include "unity/unity.h"

void test_fof_queue_chunk_handling(void);
void test_fof_queue(void);
void test_dsp_client(void);
void test_ctrl_client(void);

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}
int main (int argc, char *argv[])
{
  UNITY_BEGIN();
  RUN_TEST(test_fof_queue_chunk_handling);
  RUN_TEST(test_fof_queue);
  RUN_TEST(test_dsp_client);
  RUN_TEST(test_ctrl_client);
  return UNITY_END();
}
