#include <unistd.h>

#include "unity/unity.h"

void test_process_queue(void);
void test_dsp_client(void);

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}
int main (int argc, char *argv[])
{
  UNITY_BEGIN();
  //RUN_TEST(test_process_queue);
  RUN_TEST(test_dsp_client);
  return UNITY_END();
}
