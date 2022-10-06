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
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if(NOT TARGET RpcHpp::rpc_hpp)
  message(
    FATAL_ERROR
      "RpcHpp::rpc_hpp alias not set. Please generate CMake from the root directory"
  )
endif()

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include(CTest)
include(doctest)
include(system_link)
enable_testing()

find_package(Threads REQUIRED)

if(NOT TARGET doctest_lib)
  add_library(doctest_lib INTERFACE)
  find_package(doctest CONFIG REQUIRED)
  target_link_libraries_system(doctest_lib INTERFACE doctest::doctest)
endif()

add_executable(rpc_test)
target_link_libraries(rpc_test PRIVATE rpc_hpp doctest_lib Threads::Threads)
target_sources(
  rpc_test
  PRIVATE test_client.cpp
          test_server.cpp
          test_client.hpp
          test_server.hpp
          static_funcs.hpp
          test_structs.hpp
          ../include/rpc.hpp
          ../include/rpc_client.hpp
          ../include/rpc_server.hpp)

if(${USE_PCH})
  target_precompile_headers(rpc_test PRIVATE pch.hpp)
endif()

if(${BUILD_ADAPTER_BITSERY})
  target_link_libraries(rpc_test PRIVATE bitsery_adapter)
  target_compile_definitions(rpc_test PRIVATE RPC_HPP_BITSERY_EXACT_SZ)
  target_sources(rpc_test PRIVATE ../include/rpc_adapters/rpc_bitsery.hpp)
endif()

if(${BUILD_ADAPTER_BOOST_JSON})
  target_link_libraries(rpc_test PRIVATE boost_json_adapter)
  target_sources(rpc_test PRIVATE ../include/rpc_adapters/rpc_boost_json.hpp)
endif()

if(${BUILD_ADAPTER_NJSON})
  target_link_libraries(rpc_test PRIVATE njson_adapter)
  target_sources(rpc_test PRIVATE ../include/rpc_adapters/rpc_njson.hpp)
endif()

if(${BUILD_ADAPTER_RAPIDJSON})
  target_link_libraries(rpc_test PRIVATE rpdjson_adapter)
  target_sources(rpc_test PRIVATE ../include/rpc_adapters/rpc_rapidjson.hpp)
endif()

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
  target_link_libraries(rpc_test PRIVATE coverage_config)
endif()

target_compile_options(rpc_test PRIVATE ${FULL_WARNING})
doctest_discover_tests(rpc_test ADD_LABELS 0)
