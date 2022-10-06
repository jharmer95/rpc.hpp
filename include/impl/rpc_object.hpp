#ifndef RPC_HPP_RPC_OBJECT_HPP
#define RPC_HPP_RPC_OBJECT_HPP

#include "detail/macros.hpp"
#include "detail/rpc_types.hpp"
#include "rpc_exceptions.hpp"

#include <optional>
#include <type_traits>
#include <utility>

namespace rpc_hpp
{
enum class rpc_type : int
{
    callback_install_request,
    callback_error,
    callback_request,
    callback_result,
    callback_result_w_bind,
    func_error,
    func_request,
    func_result,
    func_result_w_bind,
};

template<typename Serial>
class rpc_object
{
public:
    using serial_t = typename Serial::serial_t;
    using bytes_t = typename Serial::bytes_t;

    RPC_HPP_NODISCARD("parsing consumes the original input")
    static std::optional<rpc_object> parse_bytes(bytes_t&& bytes)
    {
        try
        {
            return rpc_object{ Serial::from_bytes(std::move(bytes)) };
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }

    template<bool IsCallback, typename R>
    explicit rpc_object(detail::rpc_result<IsCallback, R> result)
        : m_obj(Serial::serialize_result(std::move(result)))
    {
    }

    template<bool IsCallback, typename... Args>
    explicit rpc_object(detail::rpc_request<IsCallback, Args...> request)
        : m_obj(Serial::serialize_request(std::move(request)))
    {
    }

    template<bool IsCallback>
    explicit rpc_object(detail::rpc_error<IsCallback> error)
        : m_obj(Serial::serialize_error(std::move(error)))
    {
    }

    template<bool IsCallback, typename R, typename... Args>
    explicit rpc_object(detail::rpc_result_w_bind<IsCallback, R, Args...> result)
        : m_obj(Serial::serialize_result_w_bind(std::move(result)))
    {
    }

    explicit rpc_object(callback_install_request callback_req)
        : m_obj(Serial::serialize_callback_install(std::move(callback_req)))
    {
    }

    RPC_HPP_NODISCARD("converting to bytes may be expensive")
    bytes_t to_bytes() const& { return Serial::to_bytes(m_obj); }

    RPC_HPP_NODISCARD("converting to bytes consumes object")
    bytes_t to_bytes() && { return Serial::to_bytes(std::move(m_obj)); }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    std::string get_func_name() const { return Serial::get_func_name(m_obj); }

    template<typename R>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    R get_result() const
    {
        switch (type())
        {
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
                if constexpr (std::is_void_v<R>)
                {
                    return;
                }
                else
                {
                    return Serial::template get_result<false, R>(m_obj).result;
                }

            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
                if constexpr (std::is_void_v<R>)
                {
                    return;
                }
                else
                {
                    return Serial::template get_result<true, R>(m_obj).result;
                }

            case rpc_type::func_error:
                detail::rpc_throw(Serial::template get_error<false>(m_obj));

            case rpc_type::callback_error:
                detail::rpc_throw(Serial::template get_error<true>(m_obj));

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::func_request:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    template<typename... Args>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_args() const
    {
        switch (type())
        {
            case rpc_type::func_request:
            case rpc_type::func_result_w_bind:
                return Serial::template get_request<false, Args...>(m_obj).args;

            case rpc_type::callback_request:
            case rpc_type::callback_result_w_bind:
                return Serial::template get_request<true, Args...>(m_obj).args;

            case rpc_type::callback_error:
            case rpc_type::callback_install_request:
            case rpc_type::callback_result:
            case rpc_type::func_error:
            case rpc_type::func_result:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool is_callback_uninstall() const
    {
        return type() == rpc_type::callback_install_request
            ? Serial::get_callback_install(m_obj).is_uninstall
            : false;
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    exception_type get_error_type() const
    {
        switch (type())
        {
            case rpc_type::callback_error:
                return Serial::template get_error<true>(m_obj).except_type;

            case rpc_type::func_error:
                return Serial::template get_error<false>(m_obj).except_type;

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
            case rpc_type::func_request:
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    template<bool IsCallback = false>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    std::string get_error_mesg() const
    {
        switch (type())
        {
            case rpc_type::callback_error:
                return Serial::template get_error<true>(m_obj).err_mesg;

            case rpc_type::func_error:
                return Serial::template get_error<false>(m_obj).err_mesg;

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
            case rpc_type::func_request:
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool has_bound_args() const
    {
        switch (type())
        {
            case rpc_type::func_request:
            case rpc_type::callback_request:
                return Serial::has_bound_args(m_obj);

            case rpc_type::func_result_w_bind:
            case rpc_type::callback_result_w_bind:
                return true;

            case rpc_type::callback_error:
            case rpc_type::callback_install_request:
            case rpc_type::callback_result:
            case rpc_type::func_error:
            case rpc_type::func_result:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool is_error() const
    {
        const auto rtype = type();
        return (rtype == rpc_type::func_error) || (rtype == rpc_type::callback_error);
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    rpc_type type() const { return Serial::get_type(m_obj); }

private:
    explicit rpc_object(const serial_t& serial) : m_obj(serial) {}
    explicit rpc_object(serial_t&& serial) noexcept : m_obj(std::move(serial)) {}

    serial_t m_obj;
};
} //namespace rpc_hpp

#endif
