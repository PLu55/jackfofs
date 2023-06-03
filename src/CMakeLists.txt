cmake_minimum_required (VERSION 3.24)

project (Jack_Fofs VERSION 4.0.1)

set(CMAKE_C_FLAGS "-g -Wall")
set(FOFSHOME /home/plu/Projects/fofs)

option(DEBUG_ENABLE "Enable Trace" OFF)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIB_SRCS
  shmem.c
  fof_queue.c
  api.c
  debug.c
  shmem.h
  fof_queue.h
  api.h
  debug.h)

set(SRCS
  shmem.c
  manager.c
  fof_queue.c
  ctrl_client.c
  dsp_client.c
  mix_client.c
  debug.c
)

set(HDRS
  jfofs.h
  jfofs_types.h
  jfofs_private.h
  manager.h
  fof_queue.h
  ctrl_client.h
  dsp_client.h
  debug.h
)

set(TEST_SRCS
  test_main.c
  test_util.c
  test_manager.c
  signal_tester_client.c
  sin_gen.c
  test_fof_queue.c
  test_ctrl_client.c
  test_dsp_client.c
  test_mix_client.c
  api.c
  test_api.c
)

set(TEST_HDRS
  test_util.h
  signal_tester_client.h
  sin_gen.h)

set(LIBS
  libjack.so
  libm.so
  ${FOFSHOME}/build/libfofs.so)

#add_subdirectory(tests)

include_directories(${FOFSHOME}/src /usr/local/include)

#link_libraries(libjack.so)
#link_libraries(libm.so)
#link_libraries(${FOFSHOME}/build/libfofs.so)

add_executable(jfofs)
target_sources(jfofs PRIVATE main.c ${SRCS} ${HDRS})
target_link_libraries(jfofs ${LIBS})

add_library(jfofs_api SHARED)
set_target_properties(jfofs_api
  PROPERTIES OUTPUT_NAME "jfofs")
target_sources(jfofs_api PRIVATE ${LIB_SRCS})
target_link_libraries(jfofs_api libjack.so)

add_executable(jfofs_test)
target_sources(jfofs_test PRIVATE ${TEST_SRCS} ${TEST_HDRS} ${SRCS} ${HDRS})
target_link_libraries(jfofs_test libunity.a ${LIBS})

#add_executable(shmem)
#target_sources(shmem PRIVATE
#  main_shmem.c
#  shmem.c
#  shmem.h)

add_executable(timing)
target_sources(timing PRIVATE
  test_timing.c)
target_link_libraries(timing libjack.so ${CMAKE_CURRENT_BINARY_DIR}/libjfofs.so)

add_executable(jfofs_stress)
target_sources(jfofs_stress PRIVATE
  stress_main.c
  test_util.c)
target_link_libraries(jfofs_stress ${CMAKE_CURRENT_BINARY_DIR}/libjfofs.so)
