# compile to a universal library when compiling for mac os
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# this project
cmake_minimum_required(VERSION 3.16.3)
project(eset CXX)
set(CMAKE_CXX_STANDARD 20)

# include files
include_directories(eset 
    ${PROJECT_SOURCE_DIR}/include/
)

# add source files
file(GLOB_RECURSE src-files "src/*.cpp")
file(GLOB_RECURSE include-files "include/*.h")

# add executable
add_library(eset STATIC ${src-files} ${include-files})