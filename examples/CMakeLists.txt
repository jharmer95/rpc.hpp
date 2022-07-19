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

cmake_minimum_required(VERSION 3.16)

project(
  "rpc.hpp examples"
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
  find_package(asio CONFIG REQUIRED)
  target_link_libraries(asio_lib INTERFACE asio asio::asio)
endif()

add_executable(rpc_server "tcp_server/server.cpp")
target_link_libraries(rpc_server PRIVATE rpc_hpp asio_lib njson_adapter)
target_compile_options(rpc_server PRIVATE ${FULL_WARNING})

add_executable(rpc_client "tcp_server/client.cpp")
target_link_libraries(rpc_client PRIVATE rpc_hpp asio_lib njson_adapter)
target_compile_options(rpc_client PRIVATE ${FULL_WARNING})

add_library(rpc_module MODULE "module/module.cpp")
target_link_libraries(rpc_module PRIVATE rpc_hpp njson_adapter)
target_compile_definitions(rpc_module PRIVATE -DRPC_HPP_EXPORT)
target_compile_options(rpc_module PRIVATE ${FULL_WARNING})

add_executable(rpc_module_client "module/client.cpp")
target_link_libraries(rpc_module_client PRIVATE rpc_hpp njson_adapter ${CMAKE_DL_LIBS})
target_compile_options(rpc_module_client PRIVATE ${FULL_WARNING})
