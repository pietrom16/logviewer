###############################################################################
# CMakeLists.txt - logviewer
#
# Copyright 2012-2017 Pietro Mele
###############################################################################

cmake_minimum_required(VERSION 3.0)

set(PRJ logviewer)

project(${PRJ})

message("==============================================")
message("Building project: ${PRJ}")
message("==============================================")

#------------------------------------------------------------------------------
### Project configuration

# Specify the build type on the command line: Release, Debug, RelWithDebInfo, MinSizeRel
message("TEMP - CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} MATCHES "")
	#set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_BUILD_TYPE Release)
	message("CMAKE_BUILD_TYPE set to default: ${CMAKE_BUILD_TYPE}")
else()
	message("CMAKE_BUILD_TYPE set to: ${CMAKE_BUILD_TYPE}")
endif()

# Specify C++ standard version
set(CMAKE_CXX_FLAGS "-std=c++14 -stdlib=libc++")
set(CMAKE_CXX_STANDARD 14)
add_compile_options(-std=c++14)
link_libraries("-stdlib=libc++")

# Extra code in debug mode
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	message("Building in release mode.")
else()
	message("Building extra checks and output in debug mode.")
	add_definitions(-DVERBOSE)
endif()

# Internal tests: LOGCONTEXT_TEST, READ_KEYBOARD_TEST
#add_definitions(-DRUN_INTERNAL_TESTS)
#add_definitions(-DLOGCONTEXT_TEST)
#add_definitions(-DREAD_KEYBOARD_TEST)

message("Building with: " ${CMAKE_CXX_COMPILER} " " ${CMAKE_CXX_FLAGS})

#------------------------------------------------------------------------------
### Qt library

set(QT_GUI "0")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)

if(Qt5Widgets_FOUND)
	message("Qt5 library found. Building graphic GUI interface.")
	add_definitions(-DQT_GUI)
	set(QT_GUI "1")
	# Instruct CMake to run moc automatically when needed
	set(CMAKE_AUTOMOC ON)
else()
	message("Qt5 library not found.")
	message("Searched for Qt5 here: ${CMAKE_PREFIX_PATH}")
	message("One way to fix this, if Qt is already installed, is to set the QTDIR environment ")
	message("variable and add the Qt's bin path to the system's PATH.")
	message("Building console interface only.")
endif()

#------------------------------------------------------------------------------
### Source files

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
	progArgs.cpp
	progArgs.h
	ReadKeyboard.cpp
	ReadKeyboard.h
	ReadKeyboard_test.cpp
	RunInternalTests.cpp
	RunInternalTests.h
	textModeFormatting.h
)

set(SRC_QT
	QLogViewer.cpp
	QLogViewer.hpp
)

set(DOC
	logviewer.css
	README.md
	TODO
)

set(SRC
	${SRC_CORE}
)

if(QT_GUI MATCHES "1")
	set(SRC
		${SRC_CORE}
		${SRC_QT}
	)
endif()

#------------------------------------------------------------------------------
### Build

message("Compiling: ${SRC}")

add_executable(${PRJ} ${SRC})
add_executable(${PRJ}_test test_logsGenerator.cpp)

if(Qt5Widgets_FOUND)
	target_link_libraries(${PRJ} Qt5::Widgets)
endif()

