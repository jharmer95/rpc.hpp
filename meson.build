project('rpc.hpp', 'cpp',
    version : '0.4.1',
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

thread_dep = dependency('threads')

include_dirs = include_directories('./include')
rpc_hpp_dep = declare_dependency(include_directories : include_dirs, dependencies : thread_dep)
header_list = ['include/rpc.hpp', 'include/rpc_dispatch_helper.hpp']
install_headers(header_list)

subdir('include/rpc_adapters')
subdir('tests')
