cmake_minimum_required(VERSION 3.5)
project(g5)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES g5compiler.cpp)
add_executable(g5 ${SOURCE_FILES})

enable_testing()
add_test(NAME test_consts COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/consts.go")
add_test(NAME test_import COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/import.go")
add_test(NAME test_vardecl COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/vardecl.go")
add_test(NAME test_typedecl COMMAND g5  "${PROJECT_SOURCE_DIR}/test/adhoc/typedecl.go")
