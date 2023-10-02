# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project's name
project(Chip8Emulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)

# Add compiler warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Set the -fsanitize flag for Debug builds
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

# Add executable target with source files
add_executable(Chip8Emulator src/chip8.cpp src/display.cpp src/main.cpp)

# Include header files
target_include_directories(Chip8Emulator PRIVATE ${PROJECT_SOURCE_DIR}/src)