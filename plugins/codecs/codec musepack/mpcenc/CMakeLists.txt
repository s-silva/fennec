CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
project(mppenc C)
set(CMAKE_VERBOSE_MAKEFILE false)

if(NOT MSVC)
set(CMAKE_C_FLAGS "-O2 -ffast-math -fomit-frame-pointer -pipe")
endif(NOT MSVC)

add_subdirectory(src)
