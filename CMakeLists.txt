# Specify the minimum version for CMake

cmake_minimum_required(VERSION 3.13.4)

# Project's name

project(multi_index)

# Set the output folder where your program will be created

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The following folder will be included

include_directories("${PROJECT_SOURCE_DIR}")

set(SOURCES 
	example.cpp
	allocator/chunk.cpp 
	allocator/fixed_allocator.cpp 
	allocator/small_object_allocator.cpp
)

add_executable(multi_index ${SOURCES})
