#include <stdio.h>
#include <unity/unity.h>
#include <fofs.h>
#include <jack/jack.h>
#include<unistd.h>

#include "signal_tester_client.h"
#include "manager.h"
#include "test_util.h"

void test_manager(void)
{
  manager* mgr;
  int status;

  mgr = manager_new(&status);
  TEST_ASSERT_NOT_NULL(mgr);
  TEST_ASSERT_NOT_NULL(mgr->ctrl);
  TEST_ASSERT_NOT_NULL(mgr->dsp[0]);
  //TEST_ASSERT_NOT_NULL(mgr->mix);

  sleep(5);
  
  TEST_ASSERT_TRUE(manager_activate_clients(mgr));
  TEST_ASSERT_TRUE(manager_connect_clients(mgr));
  
  TEST_ASSERT_TRUE(jack_port_connected(mgr->ctrl->port));
  TEST_ASSERT_TRUE(jack_port_connected(mgr->dsp[0]->in_port));
  
  //int 	jack_port_connected_to (const jack_port_t *port, const char *port_name) JACK_OPTIONAL_WEAK_EXPORT
}
