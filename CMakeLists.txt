cmake_minimum_required(VERSION 3.1...3.15)

project("rpc.hpp" VERSION 0.1.0.0 DESCRIPTION "Simple RPC Header-Only Library" LANGUAGES CXX)

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)

if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt)! Please create a 'build' directory.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(FULL_WARNING /W4 /experimental:external /external:W0 /external:anglebrackets
        /permissive- /wd4619 /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14545
        /w14546 /w14547 /w14549 /w14555 /w14640 /w14826 /w14905 /w14906 /w14928)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(FULL_WARNING -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast
        -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion
        -Wnull-dereference -Wdouble-promotion -Wformat=2 -Weffc++ -Wuseless-cast -Wlogical-op
        -Wduplicated-branches -Wduplicated-cond -Wmisleading-indentation -Wno-missing-prototypes)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(FULL_WARNING -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-c++11-compat
        -Wno-c++98-c++11-compat-binary-literal -Wno-c++98-c++11-c++14-compat -Wno-padded -Wno-missing-prototypes)
endif()

add_executable(rpc_test "${PROJECT_SOURCE_DIR}/tests/rpc.test.cpp")
target_include_directories(rpc_test PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_options(rpc_test PRIVATE ${FULL_WARNING})
