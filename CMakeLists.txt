cmake_minimum_required(VERSION 3.23)
project(automac C)

include_directories(include)

set(CMAKE_C_STANDARD 11)

add_executable(automac main.c src/ping.c src/mac.c src/utils.c)
