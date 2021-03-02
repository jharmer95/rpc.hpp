add_library(catch2_lib STATIC catch2_main.cpp)
add_library(asio_lib INTERFACE)

if(${USE_CONAN})
  set(CONAN_EXTRA_REQUIRES "catch2/[>2.10.0 <3.0.0]" "asio/[>=1.14.0]")
  run_conan()
  target_link_libraries(catch2_lib PUBLIC CONAN_PKG::catch2)
  target_link_libraries(asio_lib INTERFACE CONAN_PKG::asio)
elseif(${USE_VCPKG})
  find_package(Catch2 CONFIG REQUIRED)
  target_link_libraries(catch2_lib PUBLIC Catch2::Catch2)
  find_package(asio CONFIG REQUIRED)
  target_link_libraries(asio_lib INTERFACE asio asio::asio)
else()
  find_path(catch2_path catch2/catch.hpp)
  target_include_directories(catch2_lib SYSTEM PUBLIC catch2_path)
  find_path(asio_path asio.hpp)
  target_include_directories(asio_lib SYSTEM INTERFACE asio_path)
endif()

add_executable(rpc_test "rpc.test.cpp")
target_link_libraries(rpc_test PRIVATE rpc_hpp catch2_lib asio_lib)

if(${BUILD_NJSON_ADAPTER})
  target_link_libraries(rpc_test PRIVATE njson_adapter)
endif()

if(${BUILD_RAPIDJSON_ADAPTER})
  target_link_libraries(rpc_test PRIVATE rpdjson_adapter)
endif()

if(${BUILD_BOOST_JSON_ADAPTER})
  target_link_libraries(rpc_test PRIVATE boost_json_adapter)
endif()

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_link_libraries(rpc_test PRIVATE coverage_config)
endif()

target_compile_options(rpc_test PRIVATE ${FULL_WARNING})
target_compile_definitions(rpc_test PRIVATE RPC_HPP_ENABLE_POINTERS)
catch_discover_tests(rpc_test)

add_executable(rpc_benchmark "rpc.benchmark.cpp")
target_link_libraries(rpc_benchmark PRIVATE rpc_hpp catch2_lib asio_lib)

if(${BUILD_NJSON_ADAPTER})
  target_link_libraries(rpc_benchmark PRIVATE njson_adapter)
endif()

if(${BUILD_RAPIDJSON_ADAPTER})
  target_link_libraries(rpc_benchmark PRIVATE rpdjson_adapter)
endif()

if(${BUILD_BOOST_JSON_ADAPTER})
  target_link_libraries(rpc_benchmark PRIVATE boost_json_adapter)
endif()

target_compile_options(rpc_benchmark PRIVATE ${FULL_WARNING})
target_compile_definitions(rpc_benchmark PRIVATE RPC_HPP_ENABLE_POINTERS)

add_executable(rpc_server "rpc.server.cpp")
target_link_libraries(rpc_server PRIVATE rpc_hpp asio_lib)

if(${BUILD_NJSON_ADAPTER})
  target_link_libraries(rpc_server PRIVATE njson_adapter)
endif()

if(${BUILD_RAPIDJSON_ADAPTER})
  target_link_libraries(rpc_server PRIVATE rpdjson_adapter)
endif()

if(${BUILD_BOOST_JSON_ADAPTER})
  target_link_libraries(rpc_server PRIVATE boost_json_adapter)
endif()

target_compile_options(rpc_server PRIVATE ${FULL_WARNING})
target_compile_definitions(rpc_server PRIVATE RPC_HPP_ENABLE_POINTERS)
