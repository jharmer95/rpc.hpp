# BSD 3-Clause License
#
# Copyright (c) 2020-2022, Jackson Harmer
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# ==== Build Options ====

option(USE_PCH "Utilize precompiled headers" ON)
if(${CMAKE_VERSION} VERSION_LESS "3.16.0") 
  message("Please consider to switch to CMake 3.16 to take advantage of Precompiled Headers")
  set(USE_PCH OFF)
endif()

option(WERROR "Treat all warnings as errors" ON)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(GENERATE_DOXYGEN "Generate Doxygen documentation from comments" OFF)

option(BUILD_ADAPTER_BITSERY "Build the adapter for Bitsery" OFF)
if(BUILD_ADAPTER_BITSERY)
  list(APPEND VCPKG_MANIFEST_FEATURES "bitsery")
endif()

option(BUILD_ADAPTER_BOOST_JSON "Build the adapter for Boost.JSON" OFF)
if(BUILD_ADAPTER_BOOST_JSON)
  list(APPEND VCPKG_MANIFEST_FEATURES "boost-json")
endif()

option(BUILD_ADAPTER_NJSON "Build the adapter for nlohmann/json" ON)
if(BUILD_ADAPTER_NJSON)
  list(APPEND VCPKG_MANIFEST_FEATURES "nlohmann-json")
endif()

option(BUILD_ADAPTER_RAPIDJSON "Build the adapter for rapidjson" OFF)
if(BUILD_ADAPTER_RAPIDJSON)
  list(APPEND VCPKG_MANIFEST_FEATURES "rapidjson")
endif()

option(BUILD_BENCHMARK "Build the benchmarking suite" OFF)
if(BUILD_BENCHMARK)
  list(APPEND VCPKG_MANIFEST_FEATURES "benchmarks")
endif()

option(BENCH_GRPC "Include gRPC in the benchmark comparison" OFF)
if(BENCH_GRPC AND BUILD_BENCHMARK)
  list(APPEND VCPKG_MANIFEST_FEATURES "benchmarks-grpc")
endif()

option(BENCH_RPCLIB "Include rpclib in the benchmark comparison" OFF)
if(BENCH_RPCLIB AND BUILD_BENCHMARK)
  list(APPEND VCPKG_MANIFEST_FEATURES "benchmarks-rpclib")
endif()

option(BUILD_EXAMPLES "Build the examples" OFF)
if(BUILD_EXAMPLES)
  list(APPEND VCPKG_MANIFEST_FEATURES "examples")
endif()

option(BUILD_TESTING "Build the testing tree" ON)
if(BUILD_TESTING)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(
  "rpc.hpp"
  VERSION 0.8.1
  DESCRIPTION "Simple RPC Header-Only Library"
  LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==== Machine/Compiler Configs ====

if(WIN32)
  set(TARGET_WINDOWS TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_MSVC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_MINGW TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  endif()
elseif(UNIX AND NOT APPLE)
  set(TARGET_LINUX TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  endif()
elseif(APPLE)
  set(TARGET_APPLE TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  endif()
endif()

if(CXX_MSVC)
  set(FULL_WARNING
      /W4
      /experimental:external
      /external:W0
      /external:anglebrackets
      /permissive-
      /wd4619
      /w14242
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289
      /w14296
      /w14311
      /w14545
      /w14546
      /w14547
      /w14549
      /w14555
      /w14640
      /w14826
      /w14905
      /w14906
      /w14928
      /we4834
      /Zc:__cplusplus
      /bigobj
      -D_WIN32_WINNT=0x0A00)
  set(MIN_WARNING
      /W1
      /experimental:external
      /external:W0
      /external:anglebrackets
      /wd4619
      /Zc:__cplusplus
      /bigobj
      -D_WIN32_WINNT=0x0A00)
  if(WERROR)
    list(APPEND FULL_WARNING /WX)
    list(APPEND MIN_WARNING /WX)
  endif()
elseif(CXX_GCC OR CXX_MINGW)
  set(FULL_WARNING
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Weffc++
      -Wuseless-cast
      -Wlogical-op
      -Wduplicated-branches
      -Wduplicated-cond
      -Wmisleading-indentation
      -Wmissing-field-initializers)

  set(MIN_WARNING -Wall)
  if(CXX_MINGW)
    list(APPEND FULL_WARNING -Wa,-mbig-obj)
    list(APPEND MIN_WARNING -Wa,-mbig-obj)
  endif()
  if(WERROR)
    list(APPEND FULL_WARNING -Werror)
    list(APPEND MIN_WARNING -Werror)
  endif()
elseif(CXX_CLANG)
  set(FULL_WARNING
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-c++98-c++11-compat
      -Wno-c++98-c++11-compat-binary-literal
      -Wno-c++98-c++11-c++14-compat
      -Wno-padded
      -Wno-exit-time-destructors
      -Wno-missing-prototypes
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-weak-vtables
      -Wno-reserved-id-macro
      -Wno-missing-braces
      -Wno-covered-switch-default)
  set(MIN_WARNING -Wall)
  if(WERROR)
    list(APPEND FULL_WARNING -Werror)
    list(APPEND MIN_WARNING -Werror)
  endif()
endif()

# ==== Documentation Generation ====

if(${GENERATE_DOXYGEN})
  find_package(Doxygen OPTIONAL_COMPONENTS dot)

  if(DOXYGEN_FOUND)
    set(DOXYGEN_PREDEFINED __cplusplus RPC_HPP_DOXYGEN_GEN)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_HTML_OUTPUT "docs")
    set(DOXYGEN_EXCLUDE_PATTERNS */tests/* */examples/* */benchmarks/* */vcpkg/* */build/* *.py)
    set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
    doxygen_add_docs(docs ${PROJECT_SOURCE_DIR}
      COMMENT "Generate doxygen docs for rpc.hpp")
  endif()
endif()

# ==== Code Coverage ====

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

# ==== Target(s) ====

add_library(rpc_hpp INTERFACE)
add_library(RpcHpp::rpc_hpp ALIAS rpc_hpp)

include(GNUInstallDirs)
target_include_directories(rpc_hpp
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(rpc_hpp INTERFACE cxx_std_17)

# ==== Target Installation ====

include(CMakePackageConfigHelpers)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/RpcHppConfigVersion.cmake
  COMPATIBILITY SameMajorVersion)

install(TARGETS rpc_hpp
        EXPORT rpc_hpp-targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT rpc_hpp-targets
        FILE "RpcHppConfig.cmake"
        NAMESPACE RpcHpp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rpc_hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RpcHppConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rpc_hpp)
install(FILES include/rpc.hpp include/rpc_dispatch_helper.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ==== Sub-Projects ====

if(BUILD_ADAPTER_BITSERY OR BUILD_ADAPTER_BOOST_JSON OR BUILD_ADAPTER_NJSON OR BUILD_ADAPTER_RAPIDJSON)
  message("Building rpc_adapters...")
  add_subdirectory(include/rpc_adapters)
else()
  message("Skipping rpc_adapters...")
endif()

if(BUILD_EXAMPLES)
  message("Building examples...")
  add_subdirectory(examples)
else()
  message("Skipping examples...")
endif()

if(BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
  message("Building tests...")
  add_subdirectory(tests)
else()
  message("Skipping tests...")
endif()

if(BUILD_BENCHMARK)
  if(NOT BUILD_TESTING)
    message(FATAL_ERROR "Tests must be enabled for benchmarking!")
  else()
    message("Building benchmarks...")
    add_subdirectory(benchmarks)
  endif()
else()
  message("Skipping benchmarks...")
endif()
