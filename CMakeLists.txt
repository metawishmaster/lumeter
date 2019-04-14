cmake_minimum_required(VERSION 3.0.2)
project(lumeter)

find_package(Qt5Widgets)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER AutoMoc)

set(RESOURCES lumeter.qrc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()
include_directories(src/include)
add_definitions(-DLUMETER_GUI=1 -Iinclude/)
qt5_add_resources(RES_SOURCES ${RESOURCES})
#QT5_WRAP_UI( UI_HEADERS ${UIS} )
qt5_wrap_cpp(MOC_SRCS ${MOC_HEADERS})
set(lumeter_INC
				src/include/basewidget.h
				src/include/ioswidget.h
				src/include/lxapthread.h
				src/include/netstat.h
				src/include/preferences.h
				src/include/simple-cache.h
				src/include/xapdef_ext.h
				src/include/xapdef.h
				src/include/xaphub.h
	)

add_executable(lumeter ${lumeter_INC}
			src/basewidget.cpp
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
			src/xaptx.c
			src/xaphub.cpp
			${MOC_SRCS} ${RES_SOURCES} ${UI_HEADERS}
	)
target_link_libraries (lumeter Qt5::Widgets)
#set_target_properties(lumeter PROPERTIES LINK_SEARCH_END_STATIC 1)
