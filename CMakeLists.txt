cmake_minimum_required(VERSION 3.1...3.15)

project("rpc.hpp" VERSION 0.1.0.0 DESCRIPTION "Simple RPC Header-Only Library" LANGUAGES CXX)

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)

if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt)! Please create a 'build' directory.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(rpc_test "${PROJECT_SOURCE_DIR}/tests/main.cpp")
target_include_directories(rpc_test PRIVATE "${PROJECT_SOURCE_DIR}/include" "C:/include")
