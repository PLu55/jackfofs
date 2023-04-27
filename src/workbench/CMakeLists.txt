cmake_minimum_required (VERSION 3.24)

project (Jack_Fofs VERSION 0.0.1)

set(CMAKE_C_FLAGS "-g -Wall")

include_directories("/usr/local/include")

link_libraries(libjack.so)
link_libraries(libm.so)
link_libraries(libunity.a)

add_executable(jfofstest)
target_sources(jfofstest PRIVATE test_main.c test_process_queue.c
  jfofs_types.h jfofs_private.h process_queue.h)
 
