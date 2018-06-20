#cmake_minimum_required(VERSION 3.5)
project(flv_repeater)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    deps/lwlog/lwlog.h
    src/flv-repeater.c
    src/flv-repeater.h
    src/main.c src/file_utils.c)

add_executable(flv_repeater ${SOURCE_FILES} src/file_utils.c)