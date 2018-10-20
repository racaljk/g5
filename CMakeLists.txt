cmake_minimum_required(VERSION 3.5)
project(g5)

set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-fpermissive -Wunknown-pragmas")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "/sdl-")
endif()

set(SOURCE_FILES g5compiler.cpp)
add_executable(g5 ${SOURCE_FILES})

file(GLOB TEST1 ${PROJECT_SOURCE_DIR}/test/adhoc/*.go)
file(GLOB TEST2 ${PROJECT_SOURCE_DIR}/test/officialimpl/*.go)

enable_testing()
foreach(s ${TEST1})
    get_filename_component(curated ${s} NAME_WE)
    add_test(NAME adhoc_${curated} COMMAND g5  ${s})
endforeach()

foreach(s ${TEST2})
    get_filename_component(curated ${s} NAME_WE)
    add_test(NAME officialimpl_${curated} COMMAND g5  ${s})
endforeach()