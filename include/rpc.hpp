#pragma once

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace rpc_hpp
{
namespace adapters
{
    template<typename T>
    struct serial_traits;
}

template<typename... Args>
class func_request
{
public:
    func_request(std::string func_name, Args&&... args)
        : m_func_name(std::move(func_name)), m_args(std::forward_as_tuple<Args>(args)...)
    {
    }

    const std::tuple<Args...>& get_args() const { return m_args; }
    const std::string& get_func_name() const { return m_func_name; }

private:
    std::string m_func_name;
    std::tuple<Args...> m_args;
};

// response = call_func(func_request{"Sum", 1, 2});

template<typename R>
class func_result
{
public:
    func_result(std::string func_name, R result)
        : m_func_name(std::move(func_name)), m_result(std::move(result))
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const R& get_result() const { return m_result; }

private:
    std::string m_func_name;
    R m_result;
};

template<>
class func_result<void>
{
public:
    func_result(std::string func_name) : m_func_name(std::move(func_name)) {}

    const std::string& get_func_name() const { return m_func_name; }

private:
    std::string m_func_name;
};

template<typename R, typename... Args>
class func_result_w_bind
{
public:
    func_result_w_bind(std::string func_name, R result, Args&&... args)
        : m_result(std::move(result)),
          m_func_name(std::move(func_name)),
          m_args(std::forward_as_tuple<Args>(args)...)
    {
    }

private:
    R m_result;
    std::string m_func_name;
    std::tuple<Args...> m_args;
};

enum class exception_type
{
    serialization,
    deserialization,
    func_not_found,
};

// result = response.get_result<int>();

class func_error
{
public:
    func_error(std::string func_name, exception_type ex_type, std::string err_mesg)
        : m_except_type(ex_type), m_func_name(std::move(func_name)), m_err_mesg(std::move(err_mesg))
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const std::string& get_err_mesg() const { return m_err_mesg; }
    exception_type get_except_type() const { return m_except_type; }

    [[noreturn]] void rethrow() const
    {
        //switch (m_except_type)
        //{
        //}

        throw;
    }

private:
    exception_type m_except_type;
    std::string m_func_name;
    std::string m_err_mesg;
};

class callback_install
{
public:
    callback_install(std::string func_name) : m_func_name(std::move(func_name)) {}

private:
    std::string m_func_name;
};

// std::string GetPCName(int index);
// ...
// install_callback<std::string, int>(callback_install{"GetPCName", GetPCName});

enum class response_type
{
    callback_error,
    callback_install,
    callback_request,
    callback_result,
    func_error,
    func_request,
    func_result,
};

template<typename Serial>
class rpc_response
{
public:
    rpc_response(typename Serial::serial_t&& serial) : m_obj(std::move(serial)) {}

    std::string get_func_name() const { return Serial::get_func_name(m_obj); }

    template<typename R>
    R get_result() const
    {
        switch (Serial::get_type(m_obj))
        {
            case response_type::func_result:
            {
                func_result<R> res = Serial::template get_result<R>(m_obj);
                return res.get_result();
            }

            case response_type::func_error:
            {
                func_error err = Serial::get_error(m_obj);
                err.rethrow();
                break;
            }

            default:
                throw std::runtime_error("Invalid rpc_response detected");
        }
    }

    template<typename... Args>
    std::tuple<Args...> get_request() const
    {
        if (Serial::get_type(m_obj) != response_type::func_request
            && Serial::get_type(m_obj) != response_type::callback_request)
        {
            throw std::runtime_error("Invalid rpc_response detected");
        }

        func_request<Args...> req = Serial::template get_request<Args...>(m_obj);
        return req.get_args();
    }

    std::optional<func_error> get_error() const
    {
        if (Serial::get_type(m_obj) != response_type::func_error
            && Serial::get_type(m_obj) != response_type::callback_error)
        {
            return std::nullopt;
        }

        return Serial::get_error(m_obj);
    }

    response_type type() const { return Serial::get_type(m_obj); }

private:
    typename Serial::serial_t m_obj;
};

template<typename Adapter>
class serial_adapter_base
{
public:
    using serial_t = typename adapters::serial_traits<Adapter>::serial_t;
    using bytes_t = typename adapters::serial_traits<Adapter>::bytes_t;

    template<typename R>
    static func_result<R> get_result(const serial_t& serial_obj) = delete;

    template<typename R>
    static rpc_response<Adapter> serialize_result(const func_result<R>& result) = delete;

    template<typename... Args>
    static func_request<Args...> get_request(const serial_t& serial_obj) = delete;

    template<typename... Args>
    static rpc_response<Adapter> serialize_request(const func_request<Args...>& request) = delete;

    static response_type get_type(const serial_t& serial_obj) = delete;
    static func_error get_error(const serial_t& serial_obj) = delete;
    static rpc_response<Adapter> serialize_error(const func_error& error) = delete;
    static std::optional<serial_t> from_bytes(bytes_t&& bytes) = delete;
    static bytes_t to_bytes(serial_t&& serial_obj) = delete;
    static serial_t empty_object() = delete;
    static std::string get_func_name(const serial_t& serial_obj) = delete;
};
} //namespace rpc_hpp
