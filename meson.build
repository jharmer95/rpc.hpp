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

project('rpc.hpp', 'cpp',
    version : '0.6.1',
    license : 'BSD-3-Clause',
    default_options : [
        'cpp_std=c++17',
        'warning_level=3',
    ],
    meson_version : '>=0.50.0'
)

cpp_compiler = meson.get_compiler('cpp').get_id()

if cpp_compiler == 'gcc'
    project_warnings = [
        '-Wall',
        '-Wextra',
        '-Wpedantic',
        '-Wshadow',
        '-Wnon-virtual-dtor',
        '-Wold-style-cast',
        '-Wcast-align',
        '-Wunused',
        '-Woverloaded-virtual',
        '-Wconversion',
        '-Wsign-conversion',
        '-Wnull-dereference',
        '-Wdouble-promotion',
        '-Wformat=2',
        '-Weffc++',
        '-Wuseless-cast',
        '-Wlogical-op',
        '-Wduplicated-branches',
        '-Wduplicated-cond',
        '-Wmisleading-indentation'
    ]
elif cpp_compiler == 'clang'
    project_warnings = [
        '-Weverything',
        '-Wno-c++98-compat',
        '-Wno-c++98-compat-pedantic',
        '-Wno-c++98-c++11-compat',
        '-Wno-c++98-c++11-compat-binary-literal',
        '-Wno-c++98-c++11-c++14-compat',
        '-Wno-c++20-compat',
        '-Wno-padded',
        '-Wno-exit-time-destructors',
        '-Wno-missing-prototypes',
        '-Wno-documentation',
        '-Wno-documentation-unknown-command',
        '-Wno-weak-vtables',
        '-Wno-reserved-id-macro'
    ]
elif cpp_compiler == 'msvc'
    project_warnings = [
        '/W4',
        '/experimental:external',
        '/external:W0',
        '/external:anglebrackets',
        '/permissive-',
        '/wd4619',
        '/w14242',
        '/w14254',
        '/w14263',
        '/w14265',
        '/w14287',
        '/we4289',
        '/w14296',
        '/w14311',
        '/w14545',
        '/w14546',
        '/w14547',
        '/w14549',
        '/w14555',
        '/w14640',
        '/w14826',
        '/w14905',
        '/w14906',
        '/w14928',
        '/we4834',
        '/bigobj'
    ]
endif

include_dirs = include_directories('./include')
rpc_hpp_dep = declare_dependency(include_directories : include_dirs)
header_list = ['include/rpc.hpp', 'include/rpc_dispatch_helper.hpp']
install_headers(header_list)

if get_option('build_adapter_boost_json') or get_option('build_adapter_njson') or get_option('build_adapter_rapidjson')
    subdir('include/rpc_adapters')
endif

if get_option('build_benchmark')
    subdir('benchmarks')
endif

if get_option('build_examples')
    subdir('examples')
endif

if get_option('build_testing')
    subdir('tests')
endif
