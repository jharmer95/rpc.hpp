if(${BUILD_NJSON_ADAPTER})
  add_library(njson_adapter INTERFACE)
  target_include_directories(njson_adapter
                             INTERFACE "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(
    njson_adapter INTERFACE RPC_HPP_NJSON_ENABLED
                            RPC_HPP_NLOHMANN_SERIAL_TYPE=1)
  target_link_libraries(njson_adapter INTERFACE ${CMAKE_THREAD_LIBS_INIT})

  if(${USE_CONAN})
    set(CONAN_EXTRA_REQUIRES nlohmann_json/[>=3.9.0])
    run_conan()
    target_link_libraries(njson_adapter INTERFACE CONAN_PKG::nlohmann_json)
  elseif(${USE_VCPKG})
    find_package(nlohmann_json CONFIG REQUIRED)
    target_link_libraries(njson_adapter INTERFACE nlohmann_json
                                                  nlohmann_json::nlohmann_json)
  else()
    find_path(njson_path nlohmann/json.hpp)
    target_include_directories(njson_adapter SYSTEM INTERFACE njson_path)
  endif()

  install(FILES "rpc_njson.hpp"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rpc_adapters")
endif()

if(${BUILD_RAPIDJSON_ADAPTER})
  add_library(rpdjson_adapter INTERFACE)
  target_include_directories(rpdjson_adapter
                             INTERFACE "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(rpdjson_adapter
                             INTERFACE RPC_HPP_RAPIDJSON_ENABLED)
  target_link_libraries(rpdjson_adapter INTERFACE ${CMAKE_THREAD_LIBS_INIT})

  if(${USE_CONAN})
    set(CONAN_EXTRA_REQUIRES rapidjson/[>=1.1.0])
    run_conan()
    target_link_libraries(rpdjson_adapter INTERFACE CONAN_PKG::rapidjson)
  elseif(${USE_VCPKG})
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

if(${BUILD_BOOST_JSON_ADAPTER})
  add_library(boost_json_adapter INTERFACE)
  target_include_directories(boost_json_adapter
                             INTERFACE "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(boost_json_adapter
                             INTERFACE RPC_HPP_BOOST_JSON_ENABLED)
  target_link_libraries(boost_json_adapter INTERFACE ${CMAKE_THREAD_LIBS_INIT})

  if(${USE_CONAN})
    set(CONAN_EXTRA_REQUIRES boost/[>=1.75.0])
    run_conan()
    target_include_directories(boost_json_adapter SYSTEM
                               INTERFACE ${CONAN_INCLUDE_DIRS_BOOST})
  elseif(${USE_VCPKG})
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
