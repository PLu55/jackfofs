#include <unistd.h>
#include <stdio.h>
#include <unity/unity.h>

#include <fofs.h>

#include "ctrl_client.h"
#include "fof_queue.h"

void test_ctrl_client(void)
{
  ctrl_client* ctrl;
  FofMode mode = FOF_MONO;
  int nclients = 1;
  int n_fofs_per_client = 16;
  int n_preallocate_fofs = 1024;
  int status;

  ctrl = ctrl_client_new(mode, nclients, n_fofs_per_client, n_preallocate_fofs,
			 &status);
  TEST_ASSERT_NOT_NULL(ctrl);
  TEST_ASSERT_NOT_NULL(ctrl->q);
  TEST_ASSERT_EQUAL_UINT64(0, ctrl->q->next_frame);
  sleep(2);
  printf("next_frame: %ld\n", ctrl->q->next_frame);

  
}
