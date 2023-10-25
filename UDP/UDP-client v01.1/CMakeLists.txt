cmake_minimum_required(VERSION 3.0)

project(UDP-ChatRoom
	VERSION 0.0.2
	LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 11)

find_package(Threads)

set(SOURCES
    src/main.cpp
    src/udp_client.cpp
    src/udp_utilities.cpp)
add_executable(client ${SOURCES})

target_link_libraries (client Threads::Threads)

target_include_directories(client PRIVATE include)
