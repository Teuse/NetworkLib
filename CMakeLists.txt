cmake_minimum_required(VERSION 3.7.2)

project(NetworkLib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(NIBoost REQUIRED)

#--------------------------------------------------------------------
#--- Collecting all files
#--------------------------------------------------------------------
set(FILES_NET  Network/Server.h   Network/Server.cpp
               Network/Client.h   Network/Client.cpp
               Network/DataIO.h   Network/DataIO.cpp
               Network/Common.h)
source_group("Network" FILES ${FILES_NET})

#--------------------------------------------------------------------
#--- Setup Library
#--------------------------------------------------------------------
add_library(NetworkLib STATIC ${FILES_NET})

set_target_properties(NetworkLib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(NetworkLib PUBLIC .)
target_include_directories(NetworkLib PUBLIC  ${BOOST_INCLUDE_DIRS})

target_link_libraries(NetworkLib boost_signals xlibs)

if (UNIX AND NOT APPLE)
    target_link_libraries(NetworkLib pthread)
endif()
