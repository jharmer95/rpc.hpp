asio_dep = dependency('asio')
catch2_dep = dependency('catch2', version : ['>2.10.0', '<3.0.0'])

lib_catch2 = static_library('catch2', 'catch2_main.cpp', dependencies : catch2_dep)

test_dependencies = [rpc_hpp_dep, asio_dep]
test_defines = []

if get_option('build_njson_adapter')
    test_dependencies += [njson_adapter_dep]
endif

if get_option('build_rapidjson_adapter')
    test_dependencies += [rapidjson_adapter_dep]
endif

if get_option('build_boost_json_adapter')
    test_dependencies += [boost_json_adapter_dep]
endif

rpc_test_exe = executable('rpc_test', 'rpc.test.cpp', link_with : lib_catch2, dependencies : test_dependencies, install : false, cpp_args : [project_warnings, '-DRPC_HPP_ENABLE_POINTERS'])
test('rpc_test', rpc_test_exe)

rpc_bench_exe = executable('rpc_benchmark', 'rpc.benchmark.cpp', link_with : lib_catch2, dependencies : test_dependencies, install : false, cpp_args : [project_warnings, '-DRPC_HPP_ENABLE_POINTERS'])

if get_option('buildtype') in ['debugoptimized', 'release']
    benchmark('rpc_benchmark', rpc_bench_exe, args : ['--benchmark-samples 20'], timeout : 90)
endif

rpc_server_exe = executable('rpc_server', 'rpc.server.cpp', dependencies : test_dependencies, install : false, cpp_args : [project_warnings, '-DRPC_HPP_ENABLE_POINTERS'])
