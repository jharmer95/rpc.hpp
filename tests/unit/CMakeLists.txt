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
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

add_executable(ut_detail test_main.cpp detail/traits.test.cpp detail/tuple.test.cpp)
target_link_libraries(ut_detail PRIVATE rpc_hpp doctest_lib)
target_compile_options(ut_detail PRIVATE ${FULL_WARNING})
doctest_discover_tests(ut_detail ADD_LABELS 0)

add_executable(ut_exceptions test_main.cpp exceptions.test.cpp)
target_link_libraries(ut_exceptions PRIVATE rpc_hpp doctest_lib)
target_compile_options(ut_exceptions PRIVATE ${FULL_WARNING})
doctest_discover_tests(ut_exceptions ADD_LABELS 0)

add_executable(ut_rpc_obj test_main.cpp rpc_object.test.cpp)
target_link_libraries(ut_rpc_obj PRIVATE rpc_hpp doctest_lib adapters_iface)
target_compile_options(ut_rpc_obj PRIVATE ${FULL_WARNING})
doctest_discover_tests(ut_rpc_obj ADD_LABELS 0)
