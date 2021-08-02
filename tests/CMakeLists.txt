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

if(NOT TARGET RpcHpp::rpc_hpp)
  message(FATAL_ERROR "RpcHpp::rpc_hpp alias not set. Please generate CMake from the root directory")
endif()

if(NOT TARGET doctest_lib)
  add_library(doctest_lib INTERFACE)

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES "doctest/[>=2.4]")
    run_conan()
    target_link_libraries(doctest_lib INTERFACE CONAN_PKG::doctest)
  elseif(${DEPENDS_VCPKG})
    find_package(doctest CONFIG REQUIRED)
    target_link_libraries(doctest_lib INTERFACE Doctest::doctest)
  else()
    find_path(doctest_path doctest/doctest.h)
    target_include_directories(doctest_lib SYSTEM INTERFACE doctest_path)
  endif()
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

include(CTest)
include(doctest)
enable_testing()

add_executable(rpc_test "rpc.test.cpp")
target_link_libraries(rpc_test PRIVATE rpc_hpp doctest_lib asio_lib)

if(${BUILD_ADAPTER_BOOST_JSON})
  target_link_libraries(rpc_test PRIVATE boost_json_adapter)
endif()

if(${BUILD_ADAPTER_NJSON})
  target_link_libraries(rpc_test PRIVATE njson_adapter)
endif()

if(${BUILD_ADAPTER_RAPIDJSON})
  target_link_libraries(rpc_test PRIVATE rpdjson_adapter)
endif()

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_link_libraries(rpc_test PRIVATE coverage_config)
endif()

target_compile_options(rpc_test PRIVATE ${FULL_WARNING})
doctest_discover_tests(rpc_test)

find_package(Threads REQUIRED)

add_executable(test_server "rpc.server.cpp")
target_link_libraries(test_server PRIVATE rpc_hpp asio_lib Threads::Threads)

if(${BUILD_ADAPTER_BOOST_JSON})
  target_link_libraries(test_server PRIVATE boost_json_adapter)
endif()

if(${BUILD_ADAPTER_NJSON})
  target_link_libraries(test_server PRIVATE njson_adapter)
endif()

if(${BUILD_ADAPTER_RAPIDJSON})
  target_link_libraries(test_server PRIVATE rpdjson_adapter)
endif()

target_compile_options(test_server PRIVATE ${FULL_WARNING})
