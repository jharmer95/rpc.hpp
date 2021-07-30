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

if (NOT TARGET RpcHpp::rpc_hpp)
  message(FATAL_ERROR "RpcHpp::rpc_hpp alias not set. Please generate CMake from the root directory")
endif()

if(${BUILD_ADAPTER_BOOST_JSON})
  add_library(boost_json_adapter INTERFACE)
  target_include_directories(boost_json_adapter
                             INTERFACE "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(boost_json_adapter
                             INTERFACE RPC_HPP_ENABLE_BOOST_JSON)
  target_link_libraries(boost_json_adapter INTERFACE ${CMAKE_THREAD_LIBS_INIT})

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES boost/[>=1.75.0])
    run_conan()
    target_include_directories(boost_json_adapter SYSTEM
                               INTERFACE ${CONAN_INCLUDE_DIRS_BOOST})
  elseif(${DEPENDS_VCPKG})
    find_package(Boost 1.75.0 REQUIRED)
    target_include_directories(boost_json_adapter SYSTEM
                               INTERFACE ${Boost_INCLUDE_DIRS})
  else()
    find_path(boost_json_path boost/json/src.hpp)
    target_include_directories(boost_json_adapter SYSTEM
                               INTERFACE boost_json_path)
  endif()

  install(FILES "rpc_boost_json.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()

if(${BUILD_ADAPTER_NJSON})
  add_library(njson_adapter INTERFACE)
  target_include_directories(njson_adapter
                             INTERFACE "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(njson_adapter INTERFACE RPC_HPP_ENABLE_NJSON)
  target_link_libraries(njson_adapter INTERFACE ${CMAKE_THREAD_LIBS_INIT})

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES nlohmann_json/[>=3.9.0])
    run_conan()
    target_link_libraries(njson_adapter INTERFACE CONAN_PKG::nlohmann_json)
  elseif(${DEPENDS_VCPKG})
    find_package(nlohmann_json CONFIG REQUIRED)
    target_link_libraries(njson_adapter INTERFACE nlohmann_json::nlohmann_json)
  else()
    find_path(njson_path nlohmann/json.hpp)
    target_include_directories(njson_adapter SYSTEM INTERFACE njson_path)
  endif()

  install(FILES "rpc_njson.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()

if(${BUILD_ADAPTER_RAPIDJSON})
  add_library(rpdjson_adapter INTERFACE)
  target_include_directories(rpdjson_adapter
                             INTERFACE "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(rpdjson_adapter INTERFACE RPC_HPP_ENABLE_RAPIDJSON)
  target_link_libraries(rpdjson_adapter INTERFACE ${CMAKE_THREAD_LIBS_INIT})

  if(${DEPENDS_CONAN})
    set(CONAN_EXTRA_REQUIRES rapidjson/[>=1.1.0])
    run_conan()
    target_link_libraries(rpdjson_adapter INTERFACE CONAN_PKG::rapidjson)
  elseif(${DEPENDS_VCPKG})
    find_package(RapidJSON CONFIG REQUIRED)
    target_include_directories(rpdjson_adapter SYSTEM
                               INTERFACE ${RAPIDJSON_INCLUDE_DIRS})
  else()
    find_path(rpdjson_path rapidjson/document.h)
    target_include_directories(rpdjson_adapter SYSTEM INTERFACE rpdjson_path)
  endif()

  install(FILES "rpc_rapidjson.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()
