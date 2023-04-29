set(FILENAME "jack_simple_client.c")
set(SRCMAIN "main.c")
set(SRCOSC "osc.c")

cmake_minimum_required (VERSION 3.24)

project (PLu_Jack_Fofs VERSION 0.0.1)

set(CMAKE_C_FLAGS "-g -Wall")

link_libraries(libjack.so liblo.so)

add_executable(jackfofs ${SRCMAIN} ${SRCOSC})
