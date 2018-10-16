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

enable_testing()
add_test(NAME test_helloworld COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/helloworld.go")
add_test(NAME test_const COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/const.go")
add_test(NAME test_import COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/import.go")
add_test(NAME test_var COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/var.go")
add_test(NAME test_type COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/type.go")
add_test(NAME test_func COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/func.go")
add_test(NAME test_statement COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/statement.go")
