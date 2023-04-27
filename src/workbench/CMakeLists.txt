set(FILENAME "jack_simple_client.c")
set(SRC_TESTMAIN "test_main.c")
set(SRC_PROCESS_QUEUE "process_queue.c")
set(SRC_TEST_PROCESS_QUEUE "test_process_queue.c")

cmake_minimum_required (VERSION 3.24)

project (PLu_Jack_Fofs VERSION 0.0.1)

set(CMAKE_C_FLAGS "-g -Wall")

include_directories("/usr/local/include")

link_libraries(libjack.so)
link_libraries(libm.so)
link_libraries(libunity.a)

add_executable(jfofstest ${SRC_TESTMAIN} ${SRC_PROCESS_QUEUE} ${SRC_TEST_PROCESS_QUEUE})
