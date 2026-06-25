cmake_minimum_required(VERSION 3.24)

project(Jack_Fofs VERSION 4.3.1 LANGUAGES C)
message(STATUS "Building Jack_Fofs version: ${PROJECT_VERSION}")
include(CTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # static libs
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # shared libs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # executables
# target_include_directories(jfofs PRIVATE
# ${CMAKE_CURRENT_BINARY_DIR}
# )

add_subdirectory(src)
