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

asio_dep = dependency('asio')
thread_dep = dependency('threads')

ex_dependencies = [rpc_hpp_dep]
ex_defines = []

if get_option('build_adapter_boost_json')
    ex_dependencies += [boost_json_adapter_dep]
    ex_defines += ['-DRPC_HPP_ENABLE_BOOST_JSON']
endif

if get_option('build_adapter_njson')
    ex_dependencies += [njson_adapter_dep]
    ex_defines += ['-DRPC_HPP_ENABLE_NJSON']
endif

if get_option('build_adapter_rapidjson')
    ex_dependencies += [rapidjson_adapter_dep]
    ex_defines += ['-DRPC_HPP_ENABLE_RAPIDJSON']
endif

if host_machine.system() == 'linux'
    dl_dep = meson.get_compiler('cpp').find_library('dl', required: false)
else
    dl_dep = dependency('', required: false)
endif

thread_dep = dependency('threads')

rpc_server_exe = executable('rpc_server', 'server/server.cpp', dependencies : [ex_dependencies, asio_dep, thread_dep], install : false, cpp_args : [project_warnings, ex_defines])
rpc_client_exe = executable('rpc_client', 'client/client.cpp', dependencies : [ex_dependencies, asio_dep, thread_dep], install : false, cpp_args : [project_warnings, ex_defines])

rpc_module_dll = shared_module('rpc_module', 'module/module.cpp', dependencies : ex_dependencies, install : false, cpp_args : [project_warnings, ex_defines, '-DRPC_HPP_EXPORT'])
rpc_module_exe = executable('rpc_module_client', 'module/module_client.cpp', dependencies : [ex_dependencies, dl_dep], install : false, cpp_args : [project_warnings, ex_defines])
