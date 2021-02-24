asio_dep = dependency('asio', version : '>=1.14.0')
catch2_dep = dependency('catch2', version : ['>2.10.0', '<3.0.0'])

lib_catch2 = library('catch2', 'catch2_main.cpp', dependencies : catch2_dep)

rpc_test_exe = executable('rpc_test', 'rpc.test.cpp', link_with : lib_catch2, dependencies : [rpc_hpp_dep, asio_dep], install : false, cpp_args : project_warnings)
test('rpc_test', rpc_test_exe)

rpc_bench_exe = executable('rpc_benchmark', 'rpc.benchmark.cpp', link_with : lib_catch2, dependencies : [rpc_hpp_dep, asio_dep], install : false, cpp_args : project_warnings)
benchmark('rpc_benchmark', rpc_bench_exe)

rpc_server_exe = executable('rpc_server', 'rpc.server.cpp', dependencies : [rpc_hpp_dep, asio_dep], install : false, cpp_args : project_warnings)
