#ifndef RPC_EXCEPTIONS_HPP
#define RPC_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace rpc_hpp
{
enum class exception_type : int
{
    none,
    func_not_found,
    remote_exec,
    serialization,
    deserialization,
    signature_mismatch,
    client_send,
    client_receive,
    server_send,
    server_receive,
    rpc_object_mismatch,
    callback_install,
    callback_missing,
};

class rpc_exception : public std::runtime_error
{
public:
    explicit rpc_exception(const std::string& mesg, const exception_type type)
        : std::runtime_error(mesg), m_type(type)
    {
    }

    explicit rpc_exception(const char* const mesg, const exception_type type)
        : std::runtime_error(mesg), m_type(type)
    {
    }

    [[nodiscard]] exception_type get_type() const noexcept { return m_type; }

private:
    exception_type m_type;
};

class function_not_found : public rpc_exception
{
public:
    explicit function_not_found(const std::string& mesg)
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }

    explicit function_not_found(const char* const mesg)
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }
};

class remote_exec_error : public rpc_exception
{
public:
    explicit remote_exec_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }

    explicit remote_exec_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }
};

class serialization_error : public rpc_exception
{
public:
    explicit serialization_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::serialization)
    {
    }

    explicit serialization_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::serialization)
    {
    }
};

class deserialization_error : public rpc_exception
{
public:
    explicit deserialization_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }

    explicit deserialization_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }
};

class function_mismatch : public rpc_exception
{
public:
    explicit function_mismatch(const std::string& mesg)
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }

    explicit function_mismatch(const char* const mesg)
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }
};

class client_send_error : public rpc_exception
{
public:
    explicit client_send_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::client_send)
    {
    }

    explicit client_send_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::client_send)
    {
    }
};

class client_receive_error : public rpc_exception
{
public:
    explicit client_receive_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }

    explicit client_receive_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }
};

class server_send_error : public rpc_exception
{
public:
    explicit server_send_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::server_send)
    {
    }

    explicit server_send_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::server_send)
    {
    }
};

class server_receive_error : public rpc_exception
{
public:
    explicit server_receive_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }

    explicit server_receive_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }
};

class rpc_object_mismatch : public rpc_exception
{
public:
    explicit rpc_object_mismatch(const std::string& mesg)
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }

    explicit rpc_object_mismatch(const char* const mesg)
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }
};

class callback_install_error : public rpc_exception
{
public:
    explicit callback_install_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::callback_install)
    {
    }

    explicit callback_install_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::callback_install)
    {
    }
};

class callback_missing_error : public rpc_exception
{
public:
    explicit callback_missing_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::callback_missing)
    {
    }

    explicit callback_missing_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::callback_missing)
    {
    }
};
} //namespace rpc_hpp

#endif
