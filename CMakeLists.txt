cmake_minimum_required(VERSION 3.20)
project(automac C)

include_directories(Inc)

add_definitions(-DLOG_USE_COLOR)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(automac main.c Src/utils.c Src/ping.c Src/mac.c Src/log.c)
