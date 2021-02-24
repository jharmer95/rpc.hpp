if get_option('build_njson_adapter')
    njson_dep = dependency('nlohmann_json', version : '>=3.9.0')
    njson_adapter_dep = declare_dependency(dependencies : [rpc_hpp_dep, njson_dep])
    add_project_arguments(['-DRPC_HPP_NJSON_ENABLED', '-DRPC_HPP_NLOHMANN_SERIAL_TYPE=1'], language : 'cpp')
endif

if get_option('build_rapidjson_adapter')
    rapidjson_dep = dependency('rapidjson', version : '>=1.1.0')
    rapidjson_adapter_dep = declare_dependency(dependencies : [rpc_hpp_dep, rapidjson_dep])
    add_project_arguments('-DRPC_HPP_RAPIDJSON_ENABLED', language : 'cpp')
endif

adapter_list = ['rpc_njson.hpp', 'rpc_rapidjson.hpp']
install_headers(adapter_list)
