#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

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

static void test_timeout_handler(int signum)
{
    (void)signum;
    fprintf(stderr, "jfofs_test: timeout reached, aborting test run.\n");
    _exit(124);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    unsigned int timeout_s = 60;
    const char *env_timeout = getenv("JFOFS_TEST_TIMEOUT");
    if (env_timeout && env_timeout[0] != '\0')
    {
        char *endp = NULL;
        unsigned long parsed = strtoul(env_timeout, &endp, 10);
        if (endp != env_timeout)
            timeout_s = (unsigned int)parsed;
    }
    if (timeout_s > 0)
    {
        signal(SIGALRM, test_timeout_handler);
        alarm(timeout_s);
    }

    UNITY_BEGIN();
    RUN_TEST(test_fof_queue_init);
    RUN_TEST(test_fof_queue_add);
    RUN_TEST(test_fof_queue_free_list);
    RUN_TEST(test_dsp_client);
    RUN_TEST(test_ctrl_client);
    RUN_TEST(test_mix_client);
    RUN_TEST(test_manager);
    // RUN_TEST(test_api);
    // RUN_TEST(test_shmem);

    int rc = UNITY_END();
    alarm(0);
    return rc;
}
