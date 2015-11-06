# CMakeLists.txt - logviewer
#
# Copyright 2012-2015 Pietro Mele

cmake_minimum_required(VERSION 3.0)

set(PRJ logviewer)

project(${PRJ})

# Specify the build type on the command line: Release, Debug, RelWithDebInfo, MinSizeRel
if(CMAKE_BUILD_TYPE MATCHES "")
    #set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Specify compiler path
set(CMAKE_C_COMPILER /opt/local/bin/clang-mp-3.7)
set(CMAKE_CXX_COMPILER /opt/local/bin/clang++-mp-3.7)

# Specify C++ standard version
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")


message("==============================================")
message("Building project: ${PRJ}")
message("==============================================")

aux_source_directory(. SRC_LIST)
add_executable(${PRJ} ${SRC_LIST})

