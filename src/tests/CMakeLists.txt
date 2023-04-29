link_libraries(libunity.a)

add_executable(jfofstest)
#target_sources(jfofstest PRIVATE test_main.c test_process_queue.c process_queue.c 
#  jfofs_types.h jfofs_private.h process_queue.h)
 
target_include_directories(jfofstest PRIVATE ${PROJECT_SOURCE_DIR})
target_sources(jfofstest PRIVATE test_main.c test_process_queue.c
  ../process_queue.c  jfofs_types.h jfofs_private.h process_queue.h)
