cmake_minimum_required(VERSION 3.10)
project(CLIENT)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add all source files in src directory
file(GLOB SOURCES "src/*.cxx")

# Find ncurses package
find_package(Curses REQUIRED)

# Add executable target
add_executable(client ${SOURCES} main.cxx)

# Link against ncurses library
target_link_libraries(client PRIVATE ${CURSES_LIBRARIES})

# Link against threads library if needed
find_package(Threads REQUIRED)
target_link_libraries(client PRIVATE Threads::Threads)
