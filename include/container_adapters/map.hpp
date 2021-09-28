#pragma once

#include "../rpc.hpp"

#include <map>

namespace rpc::adapters
{
template<typename Key, typename T, typename Compare, typename Allocator>
class container_adapter<std::map<Key, T, Compare, Allocator>>
{
public:
    using container_t = std::map<Key, T, Compare, Allocator>;
    using citer_t = typename container_t::const_iterator;
    using iter_t = typename container_t::iterator;
    using value_t = std::pair<const Key, T>;

    explicit container_adapter(container_t& ref) noexcept : m_cont(ref) {}

    void add_element(const value_t& val) { m_cont.insert(val); }
    void add_element(value_t&& val) { m_cont.insert(std::move(val)); }

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

template<typename Key, typename T, typename Compare, typename Allocator>
struct is_map<std::map<Key, T, Compare, Allocator>> : std::true_type
{
};

template<typename Key, typename T, typename Compare, typename Allocator>
class container_adapter<std::multimap<Key, T, Compare, Allocator>>
{
public:
    using container_t = std::multimap<Key, T, Compare, Allocator>;
    using citer_t = typename container_t::const_iterator;
    using iter_t = typename container_t::iterator;
    using value_t = std::pair<const Key, T>;

    explicit container_adapter(container_t& ref) noexcept : m_cont(ref) {}

    void add_element(const value_t& val) { m_cont.insert(val); }
    void add_element(value_t&& val) { m_cont.insert(std::move(val)); }

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

template<typename Key, typename T, typename Compare, typename Allocator>
struct is_map<std::multimap<Key, T, Compare, Allocator>> : std::true_type
{
};

template<typename Key, typename T, typename Compare, typename Allocator>
struct is_multimap<std::multimap<Key, T, Compare, Allocator>> : std::true_type
{
};
} // namespace rpc::adapters
