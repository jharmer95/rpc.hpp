#pragma once

#include "../rpc.hpp"

#include <array>

namespace rpc::adapters
{
template<typename T, size_t N>
class container_adapter<std::array<T, N>>
{
public:
    using container_t = std::array<T, N>;
    using citer_t = typename container_t::const_iterator;
    using iter_t = typename container_t::iterator;
    using value_t = T;

    explicit container_adapter(container_t& ref) noexcept : m_cont(ref) {}

    void add_element(const value_t& val)
    {
        if (m_iter == end())
        {
            throw std::out_of_range("Tried to add element past array size!");
        }

        *m_iter = val;
        ++m_iter;
    }

    void add_element(value_t&& val)
    {
        if (m_iter == end())
        {
            throw std::out_of_range("Tried to add element past array size!");
        }

        *m_iter = std::move(val);
        ++m_iter;
    }

    iter_t begin() noexcept { return m_cont.begin(); }
    citer_t begin() const noexcept { return m_cont.begin(); }
    citer_t cbegin() const noexcept { return m_cont.cbegin(); }

    iter_t end() noexcept { return m_cont.end(); }
    citer_t end() const noexcept { return m_cont.end(); }
    citer_t cend() const noexcept { return m_cont.cend(); }

    iter_t next_element() & { return m_iter++; }

private:
    container_t& m_cont;
    iter_t m_iter{ std::begin(m_cont) };
};

template<typename T, size_t N>
struct is_container<std::array<T, N>> : std::true_type
{
};
} // namespace rpc::adapters
