cmake_minimum_required(VERSION 3.23)
project(automac C)

include_directories(Inc)

set(CMAKE_C_STANDARD 11)

add_executable(automac main.c Src/ping.c Src/mac.c Src/utils.c)
