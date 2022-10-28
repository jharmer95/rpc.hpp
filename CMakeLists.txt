# BSD 3-Clause License
#
# Copyright (c) 2020-2022, Jackson Harmer All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# 1. Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# 1. Neither the name of the copyright holder nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
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
  set(CMAKE_TOOLCHAIN_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# ==== Build Options ====

option(USE_PCH "Utilize precompiled headers" ON)
if(${CMAKE_VERSION} VERSION_LESS "3.16.0")
  message(
    "Please consider to switch to CMake 3.16 to take advantage of Precompiled Headers"
  )
  set(USE_PCH OFF)
endif()

option(WERROR "Treat all warnings as errors" ON)
option(SPLIT_DEBUG "Split debug information from the binary" ON)
option(GENERATE_DOXYGEN "Generate Doxygen documentation from comments" OFF)

if(NOT WIN32)
  find_program(LD_GOLD_EXECUTABLE ld.gold)
  find_program(LD_LLD_EXECUTABLE ld.lld)
  find_program(LD_MOLD_EXECUTABLE ld.mold)

  if(LD_GOLD_EXECUTABLE)
    set(DEFAULT_LINKER "gold" CACHE STRING "Preferred linker to use in builds")
  else()
    set(DEFAULT_LINKER "bfd" CACHE STRING "Preferred linker to use in builds")
  endif()

  list(APPEND FOUND_LINKERS "bfd")
  if(LD_GOLD_EXECUTABLE)
    list(APPEND FOUND_LINKERS "gold")
  endif()
  if(LD_LLD_EXECUTABLE)
    list(APPEND FOUND_LINKERS "lld")
  endif()
  if(LD_MOLD_EXECUTABLE)
    list(APPEND FOUND_LINKERS "mold")
  endif()

  set_property(CACHE DEFAULT_LINKER PROPERTY STRINGS ${FOUND_LINKERS})
endif()

find_program(CCACHE_EXECUTABLE ccache)
find_program(SCCACHE_EXECUTABLE sccache)

if(SCCACHE_EXECUTABLE)
  option(USE_SCCACHE "Utilize sccache for faster builds" ON)

  if(USE_SCCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_EXECUTABLE})
  endif()
endif()

if(CCACHE_EXECUTABLE)
  if(SCCACHE_EXECUTABLE)
    option(USE_CCACHE "Utilize ccache for faster builds" OFF)
  else()
    option(USE_CCACHE "Utilize ccache for faster builds" ON)
  endif()

  if(USE_CCACHE)
    if(USE_SCCACHE)
      message(FATAL_ERROR "USE_SCCACHE and USE_CCACHE are not compatible, choose at most one")
    endif()
    if(USE_PCH)
      set(CCACHE_ENV CCACHE_SLOPPINESS=pch_defines,time_macros)
    else()
      set(CCACHE_ENV "")
    endif()
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env ${CCACHE_ENV} ${CCACHE_EXECUTABLE})
  endif()
endif()

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
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==== Compiler/Machine Info ====

if(WIN32)
  set(TARGET_WINDOWS TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_MSVC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_MINGW TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CXX_INTEL TRUE)
  else()
    message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for Windows are: [MSVC, GNU, Clang, Intel]")
  endif()
elseif(UNIX AND NOT APPLE)
  set(TARGET_LINUX TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CXX_INTEL TRUE)
  else()
    message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for UNIX are: [GNU, Clang, Intel]")
  endif()
elseif(APPLE)
  set(TARGET_APPLE TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CXX_INTEL TRUE)
  else()
    message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for macOS are: [GNU, Clang, Intel]")
  endif()
endif()

# ==== Build Types ====

include(CheckCXXCompilerFlag)

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
set(ALLOWED_BUILD_TYPES Debug Release MinSizeRel RelWithDebInfo)

if(CXX_MSVC)
  check_cxx_compiler_flag("/fsanitize=address" SUPPORTS_ASAN)
  check_cxx_compiler_flag("/fsanitize=thread" SUPPORTS_TSAN)
else()
  list(APPEND ALLOWED_BUILD_TYPES Coverage)
  set(TMP_REQ_LIB ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address;asan")
  check_cxx_compiler_flag("-fsanitize=address" SUPPORTS_ASAN)
  set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=thread;tsan")
  check_cxx_compiler_flag("-fsanitize=thread" SUPPORTS_TSAN)
  set(CMAKE_REQUIRED_LIBRARIES ${TMP_REQ_LIB})
endif()

if(SUPPORTS_ASAN)
  list(APPEND ALLOWED_BUILD_TYPES "ASAN")
endif()

if(SUPPORTS_TSAN)
  list(APPEND ALLOWED_BUILD_TYPES "TSAN")
endif()

if(IS_MULTI_CONFIG)
  foreach(BTYPE IN LISTS ALLOWED_BUILD_TYPES)
    if (NOT ${BTYPE} IN_LIST CMAKE_CONFIGURATION_TYPES)
      list(APPEND CMAKE_CONFIGURATION_TYPES ${BTYPE})
    endif()
  endforeach()
else()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${ALLOWED_BUILD_TYPES}")
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
  elseif(NOT CMAKE_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "Unknown BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  endif()
endif()

if (CXX_CLANG OR CXX_GCC OR CXX_INTEL)
  set(GCC_COVERAGE_FLAGS "--coverage")
  set(GCC_ASAN_FLAGS "-fsanitize=address,leak,undefined -fno-omit-frame-pointer")
  set(GCC_TSAN_FLAGS "-fsanitize=thread,undefined")

  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")
  set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")
  set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")
  set(CMAKE_STATIC_LINKER_FLAGS_COVERAGE "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}" CACHE STRING "")
  set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")

  set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")
  set(CMAKE_EXE_LINKER_FLAGS_ASAN "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")
  set(CMAKE_SHARED_LINKER_FLAGS_ASAN "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")
  set(CMAKE_STATIC_LINKER_FLAGS_ASAN "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "")
  set(CMAKE_MODULE_LINKER_FLAGS_ASAN "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")

  set(CMAKE_CXX_FLAGS_TSAN ${GCC_TSAN_FLAGS} CACHE STRING "")
  set(CMAKE_EXE_LINKER_FLAGS_TSAN "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
  set(CMAKE_SHARED_LINKER_FLAGS_TSAN "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
  set(CMAKE_STATIC_LINKER_FLAGS_TSAN "${CMAKE_STATIC_LINKER_FLAGS}" CACHE STRING "")
  set(CMAKE_MODULE_LINKER_FLAGS_TSAN "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
elseif(CXX_MSVC)
  set(MSVC_ASAN_FLAGS "/fsanitize=address")

  set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${MSVC_ASAN_FLAGS}" CACHE STRING "")
  set(CMAKE_EXE_LINKER_FLAGS_ASAN ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
  set(CMAKE_SHARED_LINKER_FLAGS_ASAN ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
  set(CMAKE_STATIC_LINKER_FLAGS_ASAN ${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
  set(CMAKE_MODULE_LINKER_FLAGS_ASAN ${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
endif()

# ==== Compiler/Linker Flags ====

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
elseif(CXX_GCC OR CXX_MINGW OR CXX_INTEL)
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
      -Wno-covered-switch-default
      -Wno-unused-template
      -Wno-c++20-attribute-extensions)
  set(MIN_WARNING -Wall)
  if(WERROR)
    list(APPEND FULL_WARNING -Werror)
    list(APPEND MIN_WARNING -Werror)
  endif()
endif()

if(CXX_CLANG OR CXX_GCC OR CXX_INTEL)
  add_link_options(-fuse-ld=${DEFAULT_LINKER})
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES "MinSizeRel|Release")
  if(CXX_MSVC)
    if(NOT SPLIT_DEBUG)
      add_compile_options(/Z7)
    endif()
  elseif(SPLIT_DEBUG)
    add_compile_options(-gsplit-dwarf)
    if(NOT DEFAULT_LINKER STREQUAL "bfd")
      add_link_options("LINKER:--gdb-index")
    endif()
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
    set(DOXYGEN_EXCLUDE_PATTERNS */tests/* */examples/* */benchmarks/*
                                 */vcpkg/* */build/* *.py)
    set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
    doxygen_add_docs(docs ${PROJECT_SOURCE_DIR}
                     COMMENT "Generate doxygen docs for rpc.hpp")
  endif()
endif()

# ==== Target(s) ====

add_library(rpc_hpp INTERFACE)
add_library(RpcHpp::rpc_hpp ALIAS rpc_hpp)

include(GNUInstallDirs)
target_compile_features(rpc_hpp INTERFACE cxx_std_17)
target_include_directories(
  rpc_hpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ==== Target Installation ====

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/RpcHppConfigVersion.cmake
  COMPATIBILITY SameMajorVersion)

install(
  TARGETS rpc_hpp
  EXPORT rpc_hpp-targets
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
  EXPORT rpc_hpp-targets
  FILE "RpcHppConfig.cmake"
  NAMESPACE RpcHpp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rpc_hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RpcHppConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rpc_hpp)
install(FILES include/rpc.hpp include/rpc_dispatch_helper.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ==== Sub-Projects ====

if(BUILD_ADAPTER_BITSERY
   OR BUILD_ADAPTER_BOOST_JSON
   OR BUILD_ADAPTER_NJSON
   OR BUILD_ADAPTER_RAPIDJSON)
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
    message("Building benchmarks...")
    add_subdirectory(benchmarks)
else()
  message("Skipping benchmarks...")
endif()
