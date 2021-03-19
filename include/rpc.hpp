///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.4.0
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2021, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <tuple>
#include <type_traits>
#include <vector>

namespace rpc
{
using byte_vec = std::vector<uint8_t>;

enum class serial_t : uint8_t
{
    binary,
    json,
    unknown = 0xFFU,
};

template<serial_t Serial, typename T>
[[nodiscard]] byte_vec serialize(const T& val);

template<serial_t Serial, typename T>
[[nodiscard]] T deserialize(byte_vec&& bytes);

namespace details
{
    template<typename, typename T>
    struct is_serializable_base
    {
        static_assert(std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type");
    };

    template<typename C, typename R, typename... Args>
    struct is_serializable_base<C, R(Args...)>
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().serialize(std::declval<Args>()...)),
                R>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename, typename T>
    struct is_deserializable_base
    {
        static_assert(std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type");
    };

    template<typename C, typename R, typename... Args>
    struct is_deserializable_base<C, R(Args...)>
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().deserialize(std::declval<Args>()...)),
                R>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename Serial, typename Value>
    struct is_serializable
        : std::integral_constant<bool,
              is_serializable_base<Value, typename Serial::doc_type(const Value&)>::value
                  && is_deserializable_base<Value,
                      Value(const typename Serial::value_type&)>::value>
    {
    };

    template<typename Serial, typename Value>
    inline constexpr bool is_serializable_v = is_serializable<Serial, Value>::value;

    template<typename C>
    struct has_begin
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().begin()), typename T::iterator>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct has_end
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().end()), typename T::iterator>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct has_size
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().size()), size_t>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct is_container : std::integral_constant<bool,
                              has_size<C>::value && has_begin<C>::value && has_end<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_container_v = is_container<C>::value;

    template<typename F, typename... Ts, size_t... Is>
    void for_each_tuple(
        const std::tuple<Ts...>& tuple, const F& func, std::index_sequence<Is...> /*unused*/)
    {
        using expander = int[];
        (void)expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func)
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

    template<typename... Args>
    class packed_func_base
    {
    public:
        using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

        virtual ~packed_func_base() = default;

        packed_func_base(std::string func_name, args_t&& args) noexcept
            : m_func_name(std::move(func_name)), m_args(std::move(args))
        {
        }

        // Prevents slicing
        packed_func_base& operator=(const packed_func_base&) = delete;
        packed_func_base& operator=(packed_func_base&&) = delete;

        [[nodiscard]] std::string& get_func_name() & noexcept { return m_func_name; }
        [[nodiscard]] const std::string& get_func_name() const& noexcept { return m_func_name; }
        [[nodiscard]] std::string get_func_name() && noexcept { return std::move(m_func_name); }

        [[nodiscard]] std::string& get_err_mesg() & noexcept { return m_err_mesg; }
        [[nodiscard]] const std::string& get_err_mesg() const& noexcept { return m_err_mesg; }
        [[nodiscard]] std::string get_err_mesg() && noexcept { return std::move(m_err_mesg); }

        void set_err_mesg(const std::string& mesg) & noexcept { m_err_mesg = mesg; }
        void set_err_mesg(std::string&& mesg) & noexcept { m_err_mesg = std::move(mesg); }

        explicit virtual operator bool() const noexcept { return m_err_mesg.empty(); }

        [[nodiscard]] args_t& get_args() & noexcept { return m_args; }
        [[nodiscard]] const args_t& get_args() const& noexcept { return m_args; }
        [[nodiscard]] args_t get_args() && noexcept { return std::move(m_args); }

        template<typename T, size_t Index>
        T& get_arg() &
        {
            return std::get<Index>(m_args);
        }

        template<typename T, size_t Index>
        const T& get_arg() const&
        {
            return std::get<Index>(m_args);
        }

        template<typename T, size_t Index>
        T get_arg() &&
        {
            return std::move(std::get<Index>(m_args));
        }

        void set_args(args_t&& args) & { m_args = std::move(args); }

    protected:
        packed_func_base(const packed_func_base&) = default;
        packed_func_base(packed_func_base&&) noexcept = default;

    private:
        std::string m_func_name;
        std::string m_err_mesg{};
        args_t m_args;
    };

    template<typename R, typename... Args>
    class packed_func final : public packed_func_base<Args...>
    {
    public:
        using result_t = R;
        using typename packed_func_base<Args...>::args_t;

        packed_func(std::string func_name, std::optional<result_t> result, args_t args)
            : packed_func_base<Args...>(std::move(func_name), std::move(args)),
              m_result(std::move(result))
        {
        }

        packed_func(const packed_func&) = default;
        packed_func(packed_func&&) noexcept = default;
        packed_func& operator=(const packed_func&) & = default;
        packed_func& operator=(packed_func&&) & = default;

        explicit operator bool() const noexcept override
        {
            return m_result.has_value() && packed_func_base<Args...>::operator bool();
        }

        [[nodiscard]] R get_result() const
        {
            if (m_result.has_value())
            {
                return m_result.value();
            }

            throw std::runtime_error(this->get_err_mesg());
        }

        void set_result(R value) & noexcept { m_result = std::move(value); }
        void clear_result() & noexcept { m_result = std::nullopt; }

    private:
        std::optional<result_t> m_result{ std::nullopt };
    };

    template<typename... Args>
    class packed_func<void, Args...> final : public packed_func_base<Args...>
    {
    public:
        using result_t = void;
        using typename packed_func_base<Args...>::args_t;

        packed_func(std::string func_name, args_t args)
            : packed_func_base<Args...>(std::move(func_name), std::move(args))
        {
        }

        packed_func(const packed_func&) = default;
        packed_func(packed_func&&) noexcept = default;
        packed_func& operator=(const packed_func&) & = default;
        packed_func& operator=(packed_func&&) & = default;
    };

    template<serial_t Serial>
    class serial_adapter
    {
    public:
        template<typename R, typename... Args>
        [[nodiscard]] static byte_vec serialize_pack(const packed_func<R, Args...>& pack);

        template<typename R, typename... Args>
        [[nodiscard]] static packed_func<R, Args...> deserialize_pack(const byte_vec& bytes);

        [[nodiscard]] static std::string get_func_name(const byte_vec& bytes);
    };
} // namespace details

namespace server
{
    template<typename R, typename... Args>
    void run_callback(R (*func)(Args...), details::packed_func<R, Args...>& pack)
    {
        auto args = pack.get_args();

        if constexpr (std::is_void_v<R>)
        {
            std::apply(func, args);
            pack.set_args(std::move(args));
        }
        else
        {
            auto result = std::apply(func, args);
            pack.set_result(std::move(result));
            pack.set_args(std::move(args));
        }
    }

    template<serial_t Serial>
    void dispatch(byte_vec& bytes);

    template<serial_t Serial, typename R, typename... Args>
    void dispatch_func(R (*func)(Args...), byte_vec& bytes)
    {
        auto pack = details::serial_adapter<Serial>::template deserialize_pack<R, Args...>(bytes);
        run_callback(func, pack);
        bytes = details::serial_adapter<Serial>::serialize_pack(pack);
    }
} // namespace server

inline namespace client
{
    class client_interface
    {
    public:
        virtual ~client_interface() = default;
        virtual void send(const byte_vec& mesg) = 0;
        virtual void send(byte_vec&& mesg) = 0;
        virtual byte_vec receive() = 0;
    };

    template<serial_t Serial, typename R, typename... Args>
    R call_func(client_interface& client, std::string&& func_name, Args&&... args)
    {
        if constexpr (std::is_void_v<R>)
        {
            const details::packed_func<R, Args...> pack(
                std::move(func_name), std::make_tuple(std::forward<Args>(args)...));

            byte_vec bytes = details::serial_adapter<Serial>::serialize_pack(pack);
            client.send(std::move(bytes));
        }
        else
        {
            const details::packed_func<R, Args...> pack(
                std::move(func_name), std::nullopt, std::make_tuple(std::forward<Args>(args)...));

            byte_vec bytes = details::serial_adapter<Serial>::serialize_pack(pack);
            client.send(std::move(bytes));
        }

        const auto bytes = client.receive();
        const auto pack =
            details::serial_adapter<Serial>::template deserialize_pack<R, Args...>(bytes);

        if constexpr (std::is_void_v<R>)
        {
            if (!pack)
            {
                throw std::runtime_error(pack.get_err_mesg());
            }
            else
            {
                return;
            }
        }
        else
        {
            return pack.get_result();
        }
    }
} // namespace client
} // namespace rpc
