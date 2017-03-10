cmake_minimum_required(VERSION 3.6)
project(50_005)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        lab1/csh.c
        lab2/threadCode.c
        lab2/MedianThread_StartingCode.c)

add_executable(50_005 ${SOURCE_FILES})