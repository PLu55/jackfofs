if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.24)
  project(Jack_Fofs VERSION 4.1.1 LANGUAGES C)
endif()

# For single-config generators (e.g. Unix Makefiles), default to a reasonable
# build type when the user/preset doesn't provide one.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)

set(FOFSHOME "/home/plu/Projects/fofs" CACHE PATH "Path to the fofs checkout (used for headers)")
set(FOFS_LIB "${FOFSHOME}/build/libfofs.so" CACHE FILEPATH "Path to libfofs.so to link")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Keep build-type flags (Debug/Release) from the toolchain; only add common warnings here.
add_compile_options(-Wall)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wextra -Wpedantic)

  # Release-only performance flags (explicitly requested).
  # Note: some of these (e.g. -ffast-math, -fstrict-aliasing, -march=native)
  # can change numerical behavior and portability.
  add_compile_options(
    $<$<CONFIG:Release,RelWithDebInfo>:-O3>
    $<$<CONFIG:Release,RelWithDebInfo>:-march=native>
    $<$<CONFIG:Release,RelWithDebInfo>:-ffast-math>
    $<$<CONFIG:Release,RelWithDebInfo>:-fstrict-aliasing>
    $<$<CONFIG:Release,RelWithDebInfo>:-fno-semantic-interposition>
    $<$<CONFIG:Release,RelWithDebInfo>:-flto>
  )
  add_link_options(
    $<$<CONFIG:Release,RelWithDebInfo>:-flto>
  )
  add_compile_definitions(
    $<$<CONFIG:Release,RelWithDebInfo>:NDEBUG>
  )
endif()

option(DEBUG_ENABLE "Enable Debug" OFF)
option(STATISTICS_ENABLE "Enable Statistics" OFF)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_library(PIPEWIRE pipewire)

pkg_check_modules(SNDFILE REQUIRED sndfile)

if(PIPEWIRE)
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_PIPEWIRE")
endif()

set(LIB_SRCS
  shmem.c
  fof_queue.c
  api.c
  debug.c
  statistics.c
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
  statistics.c
  pipewire_query.c
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
  statistics.h
  pipewire_query.h
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
  test_shmem.c
)

set(TEST_HDRS
  test_util.h
  signal_tester_client.h
  sin_gen.h)

set(LIBS
  libjack.so
  libm.so
  ${FOFS_LIB})

# add_subdirectory(tests)
# Treat external headers as SYSTEM to suppress warnings coming from them.
include_directories(SYSTEM ${FOFSHOME}/src /usr/local/include /usr/include/pipewire-0.3)

# link_libraries(libjack.so)
# link_libraries(libm.so)
# link_libraries(${FOFSHOME}/build/libfofs.so)
add_executable(jfofs)
target_sources(jfofs PRIVATE main.c ${SRCS} ${HDRS})
target_link_libraries(jfofs ${LIBS})

install(TARGETS jfofs
  RUNTIME DESTINATION bin)

add_library(jfofs_api SHARED)
set_target_properties(jfofs_api
  PROPERTIES OUTPUT_NAME "jfofs")
target_sources(jfofs_api PRIVATE ${LIB_SRCS})
target_link_libraries(jfofs_api libjack.so)

install(TARGETS jfofs_api
  LIBRARY DESTINATION lib)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/jfofs.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_executable(jfofs_test)
target_sources(jfofs_test PRIVATE ${TEST_SRCS} ${TEST_HDRS} ${SRCS} ${HDRS})
target_link_libraries(jfofs_test libunity.a ${LIBS})

# add_executable(shmem)
# target_sources(shmem PRIVATE
# main_shmem.c
# shmem.c
# shmem.h)
add_executable(timing)
target_sources(timing PRIVATE
  test_timing.c)
target_link_libraries(timing libjack.so jfofs_api)

add_executable(jfofs_stress)
target_sources(jfofs_stress PRIVATE
  stress_main.c
  test_util.c)
target_link_libraries(jfofs_stress jfofs_api)

add_library(jfofs_offline STATIC)
target_sources(jfofs_offline PRIVATE offline.c offline.h)
target_include_directories(jfofs_offline SYSTEM PRIVATE ${SNDFILE_INCLUDE_DIRS} ${FOFSHOME}/src)
target_link_libraries(jfofs_offline ${SNDFILE_LIBRARIES} ${FOFS_LIB})

# Testing
include(CTest)

add_test(NAME JFOFS_MAIN_TESTING COMMAND jfofs_test)

# Prevent CI/dev runs from hanging indefinitely if JACK/PipeWire gets stuck.
# Also set an in-process watchdog via env var (can be overridden by the user).
set_tests_properties(JFOFS_MAIN_TESTING PROPERTIES
  TIMEOUT 60
  ENVIRONMENT "JFOFS_TEST_TIMEOUT=60"
)
