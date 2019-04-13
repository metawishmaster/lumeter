cmake_minimum_required(VERSION 3.0.2)
project(lumeter)

find_package(Qt5Widgets)

set(UIS lumeter.qrc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()
include_directories(.)
add_definitions(-DLUMETER_GUI=1)
add_executable(lumeter src/basewidget.cpp
           src/hub.c
           src/ioswidget.cpp
           src/lumeter.cpp
           src/lxapthread.cpp
           src/netstat.cpp
           src/preferences.cpp
           src/simple-cache.c
           src/xapinit.c
           src/xapsend.c
           src/xaprx.c
           src/xaptx.c)
target_link_libraries (lumeter Qt5::Widgets)
#set_target_properties(lumeter PROPERTIES LINK_SEARCH_END_STATIC 1)
