#include <unistd.h>

#include "unity/unity.h"

void test_process_queue(void);

int main (int argc, char *argv[])
{
  UNITY_BEGIN();
  RUN_TEST(test_process_queue);
  
  return UNITY_END();
}
