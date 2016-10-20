# CMakeLists.txt - logviewer
#
# Copyright 2012-2016 Pietro Mele

cmake_minimum_required(VERSION 3.0)

set(PRJ logviewer)

message("==============================================")
message("Building project: ${PRJ}")
message("==============================================")

project(${PRJ})

# Specify the build type on the command line: Release, Debug, RelWithDebInfo, MinSizeRel
if(CMAKE_BUILD_TYPE MATCHES "")
	set(CMAKE_BUILD_TYPE Debug)
	#set(CMAKE_BUILD_TYPE Release)
endif()

# Specify compiler path
set(CMAKE_C_COMPILER /opt/local/bin/clang-mp-3.7)
set(CMAKE_CXX_COMPILER /opt/local/bin/clang++-mp-3.7)

# Specify C++ standard version
set(CMAKE_CXX_FLAGS "-std=c++14 -stdlib=libc++")

# Extra code in debug mode
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	message("Building extra checks and output in debug mode.")
	add_definitions(-DVERBOSE)
endif()

# Internal tests: LOGCONTEXT_TEST, READ_KEYBOARD_TEST
#add_definitions(-DRUN_INTERNAL_TESTS)
#add_definitions(-DLOGCONTEXT_TEST)
#add_definitions(-DREAD_KEYBOARD_TEST)

message("Building with: " ${CMAKE_CXX_COMPILER} " " ${CMAKE_CXX_FLAGS} " " ${CMAKE_BUILD_TYPE})

set(SRC
	entrypoint.cpp
	logviewer.cpp
	logviewer_html.cpp
	logviewer.hpp
	logLevels.cpp
	logLevels.h
	LogContext.cpp
	LogContext.hpp
	LogContext_test.cpp
	LogFormatter.cpp
	LogFormatter.hpp
	progArgs.cpp
	progArgs.h
	ReadKeyboard.cpp
	ReadKeyboard.h
	ReadKeyboard_test.cpp
	RunInternalTests.cpp
	RunInternalTests.h
	textModeFormatting.h
)

add_executable(${PRJ} ${SRC})
add_executable(${PRJ}_test test_logsGenerator.cpp)

