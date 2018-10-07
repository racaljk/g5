cmake_minimum_required(VERSION 3.5)
project(g5)

set(CMAKE_CXX_STANDARD 17)

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/ DESTINATION ${CMAKE_BINARY_DIR}/)


set(SOURCE_FILES g5compiler.cpp)
add_executable(g5 ${SOURCE_FILES})

