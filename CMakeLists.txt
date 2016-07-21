#
# @file CMakeLists.txt
# @author sharpbai
# @date 2016/07/20
#

cmake_minimum_required(VERSION 2.8.4)
project(flv_repeater)

set(SOURCE_FILES src/main.c src/flv-repeater.c)

include_directories("/usr/local/include" "${PROJECT_SOURCE_DIR}/deps")

link_directories("/usr/local/lib")

add_executable(flv_repeater ${SOURCE_FILES})
