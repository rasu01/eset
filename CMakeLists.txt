# compile to a universal library when compiling for mac os
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# this project
cmake_minimum_required(VERSION 3.18)
project(bunshi CXX)
set(CMAKE_CXX_STANDARD 17)
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BUILD_PATH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BUILD_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BUILD_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BUILD_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD_PATH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BUILD_PATH}")

# include files
include_directories(bunshi 
    ${CMAKE_SOURCE_DIR}/include/
)

# add source files
file(GLOB_RECURSE src-files "src/*.cpp")
file(GLOB_RECURSE include-files "include/*.h")

# add executable
add_library(bunshi SHARED ${src-files} ${include-files})