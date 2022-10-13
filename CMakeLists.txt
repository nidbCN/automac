cmake_minimum_required(VERSION 3.23)
project(automac C)

include_directories(Inc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(automac main.c Src/ping.c Src/mac.c Src/utils.c Src/minieap.c)
