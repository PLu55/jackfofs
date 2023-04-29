cmake_minimum_required (VERSION 3.24)

project (Jack_Fofs VERSION 0.0.1)

set(CMAKE_C_FLAGS "-g -Wall")
set(FOFSHOME /home/plu/Projects/fofs)

set(SRCS
  process_queue.c
  dsp_client.c)

set(HDRS
  jfofs_types.h
  jfofs_private.h
  process_queue.h
  dsp_client.h)

set(TEST_SRCS
  test_main.c
  test_util.c
  signal_tester_client.c
  test_process_queue.c
  test_dsp_client.c)

set(TEST_HDRS
  test_util.h
  signal_tester_client.h)

#add_subdirectory(tests)

include_directories(${FOFSHOME}/src /usr/local/include)

link_libraries(libjack.so)
link_libraries(libm.so)
link_libraries(${FOFSHOME}/build/libfofs.so)


add_executable(jfofstest)
target_sources(jfofstest PRIVATE ${TEST_SRCS} ${TEST_HDRS} ${SRCS} ${HDRS})
target_link_libraries(jfofstest libunity.a)

