# this project
cmake_minimum_required(VERSION 3.18)
project(test CXX)
set(CMAKE_CXX_STANDARD 17)

# include files
include_directories(test 
    ${CMAKE_SOURCE_DIR}/../include/
)

# add eset
add_subdirectory(${CMAKE_SOURCE_DIR}/../ ${CMAKE_SOURCE_DIR}/eset/)

# add executable
add_executable(test testing.cpp)
target_link_libraries(test eset)