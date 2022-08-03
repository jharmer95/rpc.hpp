#pragma once

#include "rpc_common.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

#define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) extern RETURN FUNCNAME(__VA_ARGS__)

namespace rpc_hpp
{
///@brief Class defining an interface for serving functions via RPC
    ///
    ///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
    template<typename Serial>
    class server_interface
    {
    public:
        using adapter_t = Serial;

        server_interface() noexcept = default;

        // Prevent copying
        server_interface(const server_interface&) = delete;
        server_interface& operator=(const server_interface&) = delete;

        server_interface(server_interface&&) noexcept = default;
        server_interface& operator=(server_interface&&) noexcept = default;

#  if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        ///@brief Gets a reference to the server's function cache
        ///
        ///@tparam Val Type of the return value for a function
        ///@param func_name Name of the function to get the cached return value(s) for
        ///@return std::unordered_map<typename Serial::bytes_t, Val>& Reference to the hashmap containing the return values with the serialized function call as the key
        template<typename Val>
        std::unordered_map<typename Serial::bytes_t, Val>& get_func_cache(
            const std::string& func_name)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            update_all_cache<Val>(func_name);
            return *static_cast<std::unordered_map<typename Serial::bytes_t, Val>*>(
                m_cache_map.at(func_name));
        }

        ///@brief Clears the server's function cache
        RPC_HPP_INLINE void clear_all_cache() noexcept { m_cache_map.clear(); }
#  endif

        ///@brief Binds a string to a callback, utilizing the server's cache
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func_name Name to bind the callback to
        ///@param func_ptr Pointer to callback that runs when dispatch is called with bound name
        template<typename R, typename... Args>
        void bind_cached(std::string func_name, R (*func_ptr)(Args...))
        {
            m_dispatch_table.emplace(std::move(func_name),
                [this, func_ptr](typename Serial::serial_t& serial_obj)
                {
                    try
                    {
                        dispatch_cached_func(func_ptr, serial_obj);
                    }
                    catch (const rpc_exception& ex)
                    {
                        Serial::set_exception(serial_obj, ex);
                    }
                });
        }

        ///@brief Binds a string to a callback, utilizing the server's cache
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@tparam F Callback type (could be function or lambda or functor)
        ///@param func_name Name to bind the callback to
        ///@param func Callback to run when dispatch is called with bound name
        template<typename R, typename... Args, typename F>
        RPC_HPP_INLINE void bind_cached(std::string func_name, F&& func)
        {
            using fptr_t = R (*)(Args...);

            bind_cached(std::move(func_name), fptr_t{ std::forward<F>(func) });
        }

        ///@brief Binds a string to a callback
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func_name Name to bind the callback to
        ///@param func_ptr Pointer to callback that runs when dispatch is called with bound name
        template<typename R, typename... Args>
        void bind(std::string func_name, R (*func_ptr)(Args...))
        {
            m_dispatch_table.emplace(std::move(func_name),
                [func_ptr](typename Serial::serial_t& serial_obj)
                {
                    try
                    {
                        dispatch_func(func_ptr, serial_obj);
                    }
                    catch (const rpc_exception& ex)
                    {
                        Serial::set_exception(serial_obj, ex);
                    }
                });
        }

        ///@brief Binds a string to a callback
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@tparam F Callback type (could be function or lambda or functor)
        ///@param func_name Name to bind the callback to
        ///@param func Callback to run when dispatch is called with bound name
        template<typename R, typename... Args, typename F>
        RPC_HPP_INLINE void bind(std::string func_name, F&& func)
        {
            using fptr_t = R (*)(Args...);

            bind(std::move(func_name), fptr_t{ std::forward<F>(func) });
        }

        ///@brief Parses the received serialized data and determines which function to call
        ///
        ///@param bytes Data to be parsed into a serial object
        ///@return Serial::bytes_t Data parsed out of a serial object after dispatching the callback
        ///@note nodiscard because original bytes are consumed
        [[nodiscard]] typename Serial::bytes_t dispatch(typename Serial::bytes_t&& bytes) const
        {
            auto serial_obj = Serial::from_bytes(std::move(bytes));

            if (!serial_obj.has_value())
            {
                auto err_obj = Serial::empty_object();
                Serial::set_exception(err_obj, server_receive_error("Invalid RPC object received"));
                return Serial::to_bytes(std::move(err_obj));
            }

            const auto func_name = adapter_t::get_func_name(serial_obj.value());

            if (const auto it = m_dispatch_table.find(func_name); it != m_dispatch_table.end())
            {
                it->second(serial_obj.value());
                return Serial::to_bytes(std::move(serial_obj).value());
            }

            Serial::set_exception(serial_obj.value(),
                function_not_found("RPC error: Called function: \"" + func_name + "\" not found"));

            return Serial::to_bytes(std::move(serial_obj).value());
        }

    protected:
        ~server_interface() noexcept = default;

#  if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        template<typename R, typename... Args>
        void dispatch_cached_func(R (*func)(Args...), typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto pack = [&serial_obj]
            {
                try
                {
                    return Serial::template deserialize_pack<R, Args...>(serial_obj);
                }
                catch (const rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw deserialization_error(ex.what());
                }
            }();

            auto& result_cache = get_func_cache<R>(pack.get_func_name());

            if constexpr (!std::is_void_v<R>)
            {
                auto bytes = Serial::to_bytes(std::move(serial_obj));

                if (const auto it = result_cache.find(bytes); it != result_cache.end())
                {
                    pack.set_result(it->second);

                    try
                    {
                        serial_obj = Serial::template serialize_pack<R, Args...>(pack);
                        return;
                    }
                    catch (const rpc_exception&)
                    {
                        throw;
                    }
                    catch (const std::exception& ex)
                    {
                        throw serialization_error(ex.what());
                    }
                }

                run_callback(func, pack);
                result_cache[std::move(bytes)] = pack.get_result();
            }
            else
            {
                run_callback(func, pack);
            }

            try
            {
                serial_obj = Serial::template serialize_pack<R, Args...>(pack);
            }
            catch (const rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw serialization_error(ex.what());
            }
        }
#  else
        template<typename R, typename... Args>
        RPC_HPP_INLINE void dispatch_cached_func(
            R (*func)(Args...), typename Serial::serial_t& serial_obj) const
        {
            dispatch_func(func, serial_obj);
        }
#  endif

        template<typename R, typename... Args>
        static void dispatch_func(R (*func)(Args...), typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto pack = [&serial_obj]
            {
                try
                {
                    return Serial::template deserialize_pack<R, Args...>(serial_obj);
                }
                catch (const rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw deserialization_error(ex.what());
                }
            }();

            run_callback(func, pack);

            try
            {
                serial_obj = Serial::template serialize_pack<R, Args...>(pack);
            }
            catch (const rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw serialization_error(ex.what());
            }
        }

    private:
        template<typename R, typename... Args>
        static void run_callback(R (*func)(Args...), detail::packed_func<R, Args...>& pack)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto& args = pack.get_args();

            if constexpr (std::is_void_v<R>)
            {
                try
                {
                    std::apply(func, args);
                }
                catch (const std::exception& ex)
                {
                    throw remote_exec_error(ex.what());
                }
            }
            else
            {
                try
                {
                    auto result = std::apply(func, args);
                    pack.set_result(std::move(result));
                }
                catch (const std::exception& ex)
                {
                    throw remote_exec_error(ex.what());
                }
            }
        }

#  if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        template<typename Val>
        static void* get_func_cache_impl(const std::string& func_name)
        {
            static std::unordered_map<std::string,
                std::unordered_map<typename Serial::bytes_t, Val>>
                cache{};

            return &cache[func_name];
        }

        template<typename Val>
        void update_all_cache(const std::string& func_name)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            m_cache_map.insert_or_assign(func_name, get_func_cache_impl<Val>(func_name));
        }

        std::unordered_map<std::string, void*> m_cache_map{};
#  endif

        std::unordered_map<std::string, std::function<void(typename Serial::serial_t&)>>
            m_dispatch_table{};
    };
}
