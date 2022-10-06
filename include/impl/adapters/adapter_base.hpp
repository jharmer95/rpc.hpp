#ifndef RPC_HPP_ADAPTERS_ADAPTER_BASE_HPP
#define RPC_HPP_ADAPTERS_ADAPTER_BASE_HPP

namespace rpc_hpp::adapters
{
template<typename Adapter>
struct serial_adapter_base
{
    using bytes_t = typename Adapter::bytes_t;
    using serial_t = typename Adapter::serial_t;
    using serializer_t = typename Adapter::serializer_t;
    using deserializer_t = typename Adapter::deserializer_t;
    using config = typename Adapter::config;

    static serial_t from_bytes(bytes_t&& bytes) = delete;
    static bytes_t to_bytes(const serial_t& serial_obj) = delete;
    static bytes_t to_bytes(serial_t&& serial_obj) = delete;
    static std::string get_func_name(const serial_t& serial_obj) = delete;
    static rpc_type get_type(const serial_t& serial_obj) = delete;

    template<bool IsCallback, typename R>
    static detail::rpc_result<IsCallback, R> get_result(const serial_t& serial_obj) = delete;

    template<bool IsCallback, typename R>
    static serial_t serialize_result(const detail::rpc_result<IsCallback, R>& result) = delete;

    template<bool IsCallback, typename R, typename... Args>
    static serial_t serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) = delete;

    template<bool IsCallback, typename... Args>
    static detail::rpc_request<IsCallback, Args...> get_request(
        const serial_t& serial_obj) = delete;

    template<bool IsCallback, typename... Args>
    static serial_t serialize_request(
        const detail::rpc_request<IsCallback, Args...>& request) = delete;

    template<bool IsCallback>
    static detail::rpc_error<IsCallback> get_error(const serial_t& serial_obj) = delete;

    template<bool IsCallback>
    static serial_t serialize_error(const detail::rpc_error<IsCallback>& error) = delete;

    static callback_install_request get_callback_install(const serial_t& serial_obj) = delete;
    static serial_t serialize_callback_install(
        const callback_install_request& callback_req) = delete;

    static bool has_bound_args(const serial_t& serial_obj) = delete;
};
} //namespace rpc_hpp::adapters

#endif
