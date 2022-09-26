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

cmake_minimum_required(VERSION 3.16)

project("rpc.hpp benchmarks" LANGUAGES CXX)

if(NOT TARGET RpcHpp::rpc_hpp)
  message(
    FATAL_ERROR
      "RpcHpp::rpc_hpp alias not set. Please generate CMake from the root directory"
  )
endif()

if(NOT TARGET asio_lib)
  find_package(Threads REQUIRED)
  add_library(asio_lib INTERFACE)
  target_compile_definitions(asio_lib INTERFACE ASIO_STANDALONE)
  target_link_libraries(asio_lib INTERFACE Threads::Threads)
  find_package(asio CONFIG REQUIRED)
  target_link_libraries(asio_lib INTERFACE asio asio::asio)
endif()

if(NOT TARGET doctest_lib)
  add_library(doctest_lib INTERFACE)
  find_package(doctest CONFIG REQUIRED)
  target_link_libraries(doctest_lib INTERFACE doctest::doctest)
endif()

if(NOT TARGET nanobench_lib)
  add_library(nanobench_lib INTERFACE)
  find_path(NANOBENCH_INCLUDE_DIRS "nanobench.h")
  target_include_directories(nanobench_lib INTERFACE ${NANOBENCH_INCLUDE_DIRS})
endif()

if(${BENCH_GRPC})
  if(NOT TARGET grpc_lib)
    add_library(grpc_lib INTERFACE)
    find_package(gRPC CONFIG REQUIRED)
    target_link_libraries(grpc_lib INTERFACE gRPC::gpr gRPC::grpc gRPC::grpc++
                                             gRPC::grpc++_alts)
    target_link_libraries(grpc_lib INTERFACE re2::re2 c-ares::cares)
  endif()
endif()

if(${BENCH_RPCLIB})
  if(NOT TARGET rpclib_lib)
    add_library(rpclib_lib INTERFACE)
    find_package(rpclib CONFIG REQUIRED)
    target_link_libraries(rpclib_lib INTERFACE rpclib::rpc)
  endif()
endif()

if(${BENCH_GRPC})
  add_executable(grpc_server grpc/benchmark.pb.cc grpc/benchmark.grpc.pb.cc
                             grpc/server.cpp)
  target_link_libraries(grpc_server PRIVATE grpc_lib)

  add_library(grpc_client_obj OBJECT grpc/benchmark.pb.cc
                                     grpc/benchmark.grpc.pb.cc grpc/client.cpp)
  target_link_libraries(grpc_client_obj PUBLIC grpc_lib)
endif()

if(${BENCH_RPCLIB})
  add_executable(rpclib_server rpclib/server.cpp)
  target_link_libraries(rpclib_server PRIVATE rpclib_lib)
endif()

add_executable(rpc_benchmark benchmark.cpp)
target_compile_options(rpc_benchmark PRIVATE ${FULL_WARNING})
target_include_directories(rpc_benchmark PRIVATE ../tests)
target_link_libraries(rpc_benchmark PRIVATE rpc_hpp asio_lib doctest_lib
                                            nanobench_lib)

if(${USE_PCH})
  target_precompile_headers(rpc_benchmark PRIVATE pch.hpp)
endif()

if(${BENCH_GRPC})
  target_link_libraries(rpc_benchmark PRIVATE grpc_client_obj grpc_lib)
  target_compile_definitions(rpc_benchmark PRIVATE RPC_HPP_BENCH_GRPC)
endif()

if(${BENCH_RPCLIB})
  target_link_libraries(rpc_benchmark PRIVATE rpclib_lib)
  target_compile_definitions(rpc_benchmark PRIVATE RPC_HPP_BENCH_RPCLIB)
endif()

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
