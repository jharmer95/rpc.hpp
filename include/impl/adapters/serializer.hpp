#ifndef RPC_HPP_ADAPTERS_SERIALIZER_HPP
#define RPC_HPP_ADAPTERS_SERIALIZER_HPP

namespace rpc_hpp::adapters
{
template<typename Derived>
class generic_serializer
{
public:
    generic_serializer() noexcept = default;
    generic_serializer& operator=(const generic_serializer&) = delete;
    generic_serializer& operator=(generic_serializer&&) = delete;

    template<typename T>
    RPC_HPP_INLINE void as_bool(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_bool(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_float(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_float(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_int(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_int(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_string(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_string(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_array(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_array(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_map(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_map(key, val);
    }

    template<typename T1, typename T2>
    RPC_HPP_INLINE void as_tuple(const std::string_view key, std::pair<T1, T2>& val)
    {
        (static_cast<Derived*>(this))->as_tuple(key, val);
    }

    template<typename... Args>
    RPC_HPP_INLINE void as_tuple(const std::string_view key, std::tuple<Args...>& val)
    {
        (static_cast<Derived*>(this))->as_tuple(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_optional(const std::string_view key, std::optional<T>& val)
    {
        (static_cast<Derived*>(this))->as_optional(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_object(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_object(key, val);
    }

protected:
    ~generic_serializer() noexcept = default;
    generic_serializer(const generic_serializer&) = default;
    generic_serializer(generic_serializer&&) noexcept = default;
};

template<typename Adapter, bool Deserialize>
class serializer_base : public generic_serializer<serializer_base<Adapter, Deserialize>>
{
public:
    using serializer_t = std::conditional_t<Deserialize, typename Adapter::deserializer_t,
        typename Adapter::serializer_t>;

    template<typename T>
    void serialize_object(const T& val)
    {
        static_assert(!Deserialize, "Cannot call serialize_object() on a deserializer");

        // Necessary for bi-directional serialization
        serialize(*this, const_cast<T&>(val)); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    template<typename T>
    void deserialize_object(T&& val)
    {
        static_assert(Deserialize, "Cannot call deserialize_object() on a serializer");
        serialize(*this, std::forward<T>(val));
    }

    template<typename T>
    RPC_HPP_INLINE void as_bool(const std::string_view key, T& val)
    {
        static_assert(detail::is_boolean_testable_v<T>, "T must be convertible to bool");
        (static_cast<serializer_t*>(this))->as_bool(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_float(const std::string_view key, T& val)
    {
        static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");
        (static_cast<serializer_t*>(this))->as_float(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_int(const std::string_view key, T& val)
    {
        static_assert(std::is_integral_v<T> || std::is_enum_v<T>, "T must be an integral type");
        (static_cast<serializer_t*>(this))->as_int(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_string(const std::string_view key, T& val)
    {
        static_assert(detail::is_stringlike_v<T>, "T must be convertible to std::string_view");
        (static_cast<serializer_t*>(this))->as_string(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_array(const std::string_view key, T& val)
    {
        static_assert(detail::is_container_v<T>, "T must have begin() and end()");
        (static_cast<serializer_t*>(this))->as_array(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_map(const std::string_view key, T& val)
    {
        static_assert(detail::is_map_v<T>, "T must be a map type");

        if constexpr (detail::is_multimap_v<T>)
        {
            (static_cast<serializer_t*>(this))->as_multimap(key, val);
        }
        else
        {
            (static_cast<serializer_t*>(this))->as_map(key, val);
        }
    }

    template<typename T1, typename T2>
    RPC_HPP_INLINE void as_tuple(const std::string_view key, std::pair<T1, T2>& val)
    {
        (static_cast<serializer_t*>(this))->as_tuple(key, val);
    }

    template<typename... Args>
    RPC_HPP_INLINE void as_tuple(const std::string_view key, std::tuple<Args...>& val)
    {
        (static_cast<serializer_t*>(this))->as_tuple(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_optional(const std::string_view key, std::optional<T>& val)
    {
        (static_cast<serializer_t*>(this))->as_optional(key, val);
    }

    template<typename T>
    RPC_HPP_INLINE void as_object(const std::string_view key, T& val)
    {
        (static_cast<serializer_t*>(this))->as_object(key, val);
    }
};
} //namespace rpc_hpp::adapters

#endif
