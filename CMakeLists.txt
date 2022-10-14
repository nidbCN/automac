cmake_minimum_required(VERSION 3.20)
project(automac C)

include_directories(Inc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-static")

SET(CMAKE_C_COMPILER "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-musl-gcc")
SET(CMAKE_CXX_COMPILER "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-g++")
SET(CMAKE_AR "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-musl-ar")
SET(CMAKE_LINKER "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-musl-ld")
SET(CMAKE_RANLIB "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-ranlib")
SET(CMAKE_NM "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-nm")
SET(CMAKE_OBJDUMP "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-objdump")
SET(CMAKE_OBJCOPY "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-objcopy")
SET(CMAKE_STRIP "/usr/local/lib/openwrt-sdk-22.03.1-x86-generic_gcc-11.2.0_musl.Linux-x86_64/staging_dir/toolchain-i386_pentium4_gcc-11.2.0_musl/bin/i486-openwrt-linux-strip")

add_executable(automac main.c Src/utils.c Src/ping.c Src/mac.c Src/minieap.c)
