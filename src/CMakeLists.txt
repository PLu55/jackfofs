cmake_minimum_required (VERSION 3.24)

project (Jack_Fofs VERSION 4.0.1)

set(CMAKE_C_FLAGS "-g -Wall")
set(FOFSHOME /home/plu/Projects/fofs)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#set(LIB_SRCS
#  api.c)

set(SRCS
  shmem.c
  #manager.c
  fof_queue.c
  #ctrl_client.c
  #dsp_client.c
  #mix_client.c
)

set(HDRS
  jfofs.h
  jfofs_types.h
  jfofs_private.h
  #manager.h
  fof_queue.h
  #ctrl_client.h
  #dsp_client.h
)

set(TEST_SRCS
  test_main.c
  test_util.c
  signal_tester_client.c
  sin_gen.c
  test_fof_queue.c
  #test_ctrl_client.c
  #test_dsp_client.c
  #test_manager.c
  #test_mix_client.c
  #api.c
  #test_api.c
)

set(TEST_HDRS
  test_util.h
  signal_tester_client.h
  sin_gen.h)

#add_subdirectory(tests)

include_directories(${FOFSHOME}/src /usr/local/include)

link_libraries(libjack.so)
link_libraries(libm.so)
link_libraries(${FOFSHOME}/build/libfofs.so)


add_executable(jfofstest)
target_sources(jfofstest PRIVATE ${TEST_SRCS} ${TEST_HDRS} ${SRCS} ${HDRS})
target_link_libraries(jfofstest libunity.a)

#add_executable(jfofs)
#target_sources(jfofs PRIVATE main.c ${SRCS} ${HDRS})

#add_executable(shmem)
#target_sources(shmem PRIVATE
#  main_shmem.c
#  shmem.c
#  shmem.h)
