# BSD 3-Clause License
#
# Copyright (c) 2020-2021, Jackson Harmer
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

cmake_minimum_required(VERSION 3.16)

project(
  "rpc.hpp benchmarks"
  LANGUAGES CXX
)

if(NOT TARGET RpcHpp::rpc_hpp)
  message(FATAL_ERROR "RpcHpp::rpc_hpp alias not set. Please generate CMake from the root directory")
endif()

if(NOT TARGET asio_lib)
  find_package(Threads REQUIRED)
  add_library(asio_lib INTERFACE)
  target_compile_definitions(asio_lib INTERFACE ASIO_STANDALONE)
  target_link_libraries(asio_lib INTERFACE Threads::Threads)

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES "asio/[>=1.14.0]")
    run_conan()
    target_link_libraries(asio_lib INTERFACE CONAN_PKG::asio)
  elseif(${DEPENDS_VCPKG})
    find_package(asio CONFIG REQUIRED)
    target_link_libraries(asio_lib INTERFACE asio asio::asio)
  else()
    find_path(asio_path asio.hpp)
    target_include_directories(asio_lib SYSTEM INTERFACE asio_path)
  endif()
endif()

if(NOT TARGET doctest_lib)
  add_library(doctest_lib INTERFACE)

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES "doctest/[>=2.4]")
    run_conan()
    target_link_libraries(doctest_lib INTERFACE CONAN_PKG::doctest)
  elseif(${DEPENDS_VCPKG})
    find_package(doctest CONFIG REQUIRED)
    target_link_libraries(doctest_lib INTERFACE doctest::doctest)
  else()
    find_path(doctest_path doctest/doctest.h)
    target_include_directories(doctest_lib SYSTEM INTERFACE doctest_path)
  endif()
endif()

if(NOT TARGET nanobench_lib)
  add_library(nanobench_lib INTERFACE)

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES "nanobench/[>=4.3.0]")
    run_conan()
    target_link_libraries(nanobench_lib INTERFACE CONAN_PKG::nanobench)
  elseif(${DEPENDS_VCPKG})
    find_path(NANOBENCH_INCLUDE_DIRS "nanobench.h")
    target_include_directories(nanobench_lib SYSTEM INTERFACE ${NANOBENCH_INCLUDE_DIRS})
  else()
    find_path(nanobench_path nanobench.h)
    target_include_directories(nanobench_lib SYSTEM INTERFACE nanobench_path)
  endif()
endif()

if(NOT TARGET grpc_lib)
  add_library(grpc_lib INTERFACE)

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES "grpc/[>=1.42.0]")
    run_conan()
    target_link_libraries(grpc_lib INTERFACE CONAN_PKG::grpc)
  elseif(${DEPENDS_VCPKG})
    find_package(gRPC CONFIG REQUIRED)
    target_link_libraries(grpc_lib INTERFACE gRPC::gpr gRPC::upb gRPC::grpc gRPC::grpc++)
    target_link_libraries(grpc_lib INTERFACE re2::re2 c-ares::cares)
  else()
    find_package(Protobuf CONFIG REQUIRED)
    find_package(gRPC CONFIG REQUIRED)
    target_link_libraries(grpc_lib INTERFACE gRPC::grpc++ protobuf::libprotobuf)
  endif()
endif()

if(NOT TARGET rpclib_lib)
  add_library(rpclib_lib INTERFACE)

  if(${DEPENDS_VCPKG})
    find_package(rpclib CONFIG REQUIRED)
    target_link_libraries(rpclib_lib INTERFACE rpclib::rpc)
  else()
    find_path(rpclib_path rpc/server.h)
    find_library(rpclib_lib_path NAMES rpc librpc rpclib)
    target_include_directories(rpclib_lib SYSTEM INTERFACE rpclib_path)
    target_link_libraries(rpclib_lib INTERFACE rpclib_lib_path)
  endif()
endif()

add_executable(rpclib_server rpclib/server.cpp)
target_link_libraries(rpclib_server PRIVATE rpclib_lib)

add_executable(grpc_server grpc/benchmark.pb.cc grpc/benchmark.grpc.pb.cc grpc/server.cpp)
target_link_libraries(grpc_server PRIVATE grpc_lib)

add_executable(rpc_benchmark benchmark.cpp grpc/benchmark.pb.cc grpc/benchmark.grpc.pb.cc grpc/client.cpp)
target_include_directories(rpc_benchmark PRIVATE ../tests)
target_link_libraries(rpc_benchmark PRIVATE rpc_hpp asio_lib doctest_lib nanobench_lib grpc_lib rpclib_lib)
target_precompile_headers(rpc_benchmark PRIVATE pch.hpp)

if(${BUILD_ADAPTER_BITSERY})
  target_link_libraries(rpc_benchmark PRIVATE bitsery_adapter)
  target_compile_definitions(rpc_benchmark PRIVATE RPC_HPP_BITSERY_EXACT_SZ)
endif()

if(${BUILD_ADAPTER_BOOST_JSON})
  target_link_libraries(rpc_benchmark PRIVATE boost_json_adapter)
endif()

if(${BUILD_ADAPTER_NJSON})
  target_link_libraries(rpc_benchmark PRIVATE njson_adapter)
endif()

if(${BUILD_ADAPTER_RAPIDJSON})
  target_link_libraries(rpc_benchmark PRIVATE rpdjson_adapter)
endif()

target_compile_options(rpc_benchmark PRIVATE ${FULL_WARNING})
