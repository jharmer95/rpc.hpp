#pragma once

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

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
    func_result(std::string func_name, R&& result)
        : m_func_name(std::move(func_name)), m_result(std::forward<R>(result))
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const R& get_result() const { return m_result; }

private:
    std::string m_func_name;
    R m_result;
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
    func_error(std::string func_name, excep ex_type, std::string err_mesg)
        : m_except_type(ex_type), m_func_name(std::move(func_name)), m_err_mesg(std::move(err_mesg))
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const std::string& get_err_mesg() const { return m_err_mesg; }

    [[noreturn]] void rethrow()
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
    func_request,
    func_result,
    func_error,
    callback_install,
};

template<typename Serial>
class rpc_response
{
public:
    rpc_response(typename Serial::serial_t&& serial) : m_obj(std::move(serial)) {}

    std::string get_func_name() const { return Serial::func_name(m_obj); }

    template<typename R>
    R get_result() const
    {
        switch (Serial::get_type(m_obj))
        {
            case response_type::func_result:
                func_result<R> res = Serial::template get_result<R>(m_obj);
                return res.get_result();

            case response_type::func_error:
                func_error err = Serial::get_error(m_obj);
                err.rethrow();
                break;

            case response_type::func_request:
            case response_type::callback_install:
            default:
                throw std::runtime_error("Invalid rpc_response detected");
                break;
        }
    }

    template<typename... Args>
    std::tuple<Args...> get_request() const
    {
        if (Serial::get_type(m_obj) != response_type::func_request)
        {
            throw std::runtime_error("Invalid rpc_response detected");
        }

        func_request<Args...> req = Serial::template get_request<Args...>(m_obj);
        return req.get_args();
    }

    std::optional<func_error> get_error() const
    {
        if (Serial::get_type(m_obj) != response_type::func_error)
        {
            return std::nullopt;
        }

        return Serial::get_error(m_obj);
    }

    response_type type() const { return Serial::get_type(m_obj); }

private:
    typename Serial::serial_t m_obj;
};

// client
template<typename Serial>
void send(typename Serial::bytes_t&& bytes)
{
}

template<typename Serial>
typename Serial::bytes_t receive()
{
    return {};
}

template<typename Serial, typename... Args>
rpc_response<Serial> call_func(std::string func_name, Args&&... args)
{
    func_request<Args...> req{ std::move(func_name), std::forward<Args...>(args) };
    send<Serial>(Serial::to_bytes(Serial::serialize(req)));

    while (true)
    {
        rpc_response<Serial> resp = Serial::from_bytes(receive<Serial>());

        switch (resp.type())
        {
            case response_type::func_result:
            case response_type::func_error:
                return resp;

            case response_type::func_request:
                rpc_response<Serial> resp2 = dispatch_callback<Serial>(resp);
                assert(resp2.type() == response_type::func_result);
                send<Serial>(Serial::to_bytes(Serial::serialize(resp2)));
                break;

            case response_type::callback_install:
            default:
                throw std::runtime_error("Invalid rpc_response detected");
        }
    }
}

// server
