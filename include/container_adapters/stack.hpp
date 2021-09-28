#pragma once

#include "../rpc.hpp"

#include <stack>

namespace rpc::adapters
{
template<typename T, typename Container>
class container_adapter<std::stack<T, Container>>
{
public:
    using container_t = std::stack<T, Container>;
    using citer_t = typename container_t::const_iterator;
    using iter_t = typename container_t::iterator;
    using value_t = T;

    explicit container_adapter(container_t& ref) noexcept : m_cont(ref) {}

    void add_element(const value_t& val) { m_cont.push(val); }
    void add_element(value_t&& val) { m_cont.push(std::move(val)); }

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

template<typename T, typename Container>
struct is_container<std::stack<T, Container>> : std::true_type
{
};
} // namespace rpc::adapters
