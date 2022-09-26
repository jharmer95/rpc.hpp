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

if(NOT TARGET RpcHpp::rpc_hpp)
  message(
    FATAL_ERROR
      "RpcHpp::rpc_hpp alias not set. Please generate CMake from the root directory"
  )
endif()

include(system_link)

if(${BUILD_ADAPTER_BITSERY})
  add_library(bitsery_adapter INTERFACE)
  target_compile_definitions(bitsery_adapter INTERFACE RPC_HPP_ENABLE_BITSERY)
  find_package(Bitsery CONFIG REQUIRED)
  target_link_libraries_system(bitsery_adapter INTERFACE Bitsery::bitsery)

  install(FILES "rpc_bitsery.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()

if(${BUILD_ADAPTER_BOOST_JSON})
  add_library(boost_json_adapter INTERFACE)
  target_compile_definitions(boost_json_adapter
                             INTERFACE RPC_HPP_ENABLE_BOOST_JSON)
  find_package(Boost 1.75.0 REQUIRED COMPONENTS json)
  target_link_libraries_system(boost_json_adapter INTERFACE Boost::headers
                               Boost::json)

  install(FILES "rpc_boost_json.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()

if(${BUILD_ADAPTER_NJSON})
  add_library(njson_adapter INTERFACE)
  target_compile_definitions(njson_adapter INTERFACE RPC_HPP_ENABLE_NJSON)
  find_package(nlohmann_json CONFIG REQUIRED)
  target_link_libraries_system(njson_adapter INTERFACE
                               nlohmann_json::nlohmann_json)

  install(FILES "rpc_njson.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()

if(${BUILD_ADAPTER_RAPIDJSON})
  add_library(rpdjson_adapter INTERFACE)
  target_compile_definitions(rpdjson_adapter INTERFACE RPC_HPP_ENABLE_RAPIDJSON)
  find_package(RapidJSON CONFIG REQUIRED)
  target_include_directories(rpdjson_adapter SYSTEM
                             INTERFACE ${RAPIDJSON_INCLUDE_DIRS})

  install(FILES "rpc_rapidjson.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()
