# =============================================================================
# CMakefile for Lab-Project-FreeRTOS-POSIX
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2022 - Bithium S.A - All rights reserved.
# =============================================================================

cmake_minimum_required(VERSION 3.16.3)

project(FreeRTOS-POSIX VERSION 1.1.0)

include(CheckIncludeFile)
check_include_file("time.h" HAVE_TIME_H)

# =============================================================================
# Library
# =============================================================================

set(PHTREAD_PATH FreeRTOS-Plus-POSIX)
set(PHTREAD_SRCS_PATH ${PHTREAD_PATH}/source)

file(GLOB PTHREAD_SRCS ${PHTREAD_SRCS_PATH}/*.c)
file(GLOB_RECURSE PTHREAD_HDRS ${PHTREAD_PATH}/**/*.h)
add_library(pthread ${PTHREAD_SRCS} ${PTHREAD_HDRS})
target_include_directories(pthread
  PUBLIC
  include
  ${PHTREAD_PATH}/include
  ${PHTREAD_PATH}/include/portable
  PRIVATE
  include/private
)
target_compile_definitions(pthread
  PUBLIC
  FREERTOS_POSIX_INCLUDE_TIME_H=$<BOOL:${HAVE_TIME_H}>
)
target_link_libraries(pthread freertos_kernel)

# =============================================================================
# Installation
# =============================================================================

install(TARGETS pthread)

install(DIRECTORY include/FreeRTOS_POSIX DESTINATION include)

file(GLOB FREERTOS_POSIX_HDRS FreeRTOS-Plus-POSIX/include/*.h)
install(FILES ${FREERTOS_POSIX_HDRS} DESTINATION include)

file(GLOB FREERTOS_POSIX_PORTABLE_HDRS FreeRTOS-Plus-POSIX/include/portable/*.h)
install(FILES ${FREERTOS_POSIX_PORTABLE_HDRS} DESTINATION include)
