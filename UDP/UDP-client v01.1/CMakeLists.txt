cmake_minimum_required(VERSION 3.0)

project(UDP-ChatRoom
	VERSION 0.0.2
	LANGUAGES CXX)

set(SOURCES
    src/main.cpp
    src/udp_client.cpp
    src/udp_utilities.cpp)
add_executable(client ${SOURCES})

target_include_directories(client PRIVATE include)
