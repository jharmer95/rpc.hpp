add_library(catch2_lib STATIC catch2_main.cpp)

if(${USE_CONAN})
  set(CONAN_EXTRA_REQUIRES catch2/[>2.10.0])
  run_conan()
  target_link_libraries(catch2_lib PRIVATE CONAN_PKG::catch2)
else()
  find_path(catch2_path catch2/catch.hpp)
  target_include_directories(catch2_lib PUBLIC catch2_path)
endif()

add_executable(rpc_test "rpc.test.cpp")
target_link_libraries(rpc_test PRIVATE rpc_hpp catch2_lib)
if(${BUILD_NJSON_ADAPTER})
  target_link_libraries(rpc_test PRIVATE njson_adapter)
endif()
if(${BUILD_RAPIDJSON_ADAPTER})
  target_link_libraries(rpc_test PRIVATE rpdjson_adapter)
endif()
target_compile_options(rpc_test PRIVATE ${FULL_WARNING})
catch_discover_tests(rpc_test)

#add_executable(rpc_benchmark "rpc.benchmark.cpp")
#target_link_libraries(rpc_benchmark rpc_hpp catch2_lib)
#target_compile_options(rpc_benchmark PRIVATE ${MIN_WARNING} -D NOTHROW)
#catch_discover_tests(rpc_benchmark)