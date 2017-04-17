# CMakeLists.txt - logviewer
#
# Copyright 2012-2017 Pietro Mele

cmake_minimum_required(VERSION 3.0)

set(PRJ logviewer)

message("==============================================")
message("Building project: ${PRJ}")
message("==============================================")

project(${PRJ})

# Specify the build type on the command line: Release, Debug, RelWithDebInfo, MinSizeRel
if(CMAKE_BUILD_TYPE MATCHES "")
	#set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_BUILD_TYPE Release)
endif()

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


### Qt library

find_package(Qt5Widgets)

if(Qt5Widgets)
	message("Qt5 library found. Building graphic GUI interface.")
	add_definitions(-DQT_GUI)
	# Find includes in corresponding build directories
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	# Instruct CMake to run moc automatically when needed
	set(CMAKE_AUTOMOC ON)
else()
	message("Qt5 library not found. Building console interface only.")
endif()


set(SRC_CORE
	CSS_default.h
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
	LogFormatter_html.cpp
	LogFormatter.hpp
	logviewer.css
	progArgs.cpp
	progArgs.h
	ReadKeyboard.cpp
	ReadKeyboard.h
	ReadKeyboard_test.cpp
	README.md
	RunInternalTests.cpp
	RunInternalTests.h
	textModeFormatting.h
	TODO
)

set(SRC_QT
	QLogViewer.cpp
	QLogViewer.hpp
)

set(SRC
	${SRC_CORE}
)

if(Qt5Widgets)
	set(SRC
		${SRC_CORE}
		${SRC_QT}
	)
endif()

add_executable(${PRJ} ${SRC})
add_executable(${PRJ}_test test_logsGenerator.cpp)

if(Qt5Widgets)
	# Use the Widgets module from Qt 5
	target_link_libraries(${PRJ} Qt5::Widgets)
endif()

