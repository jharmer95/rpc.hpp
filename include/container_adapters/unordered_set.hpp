#pragma once

#include "../rpc.hpp"

#include <unordered_set>

namespace rpc::adapters
{
template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class container_adapter<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
{
public:
    using container_t = std::unordered_set<Key, Hash, KeyEqual, Allocator>;
    using citer_t = typename container_t::const_iterator;
    using iter_t = typename container_t::iterator;
    using value_t = Key;

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

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
struct is_container<std::unordered_set<Key, Hash, KeyEqual, Allocator>> : std::true_type
{
};

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class container_adapter<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
{
public:
    using container_t = std::unordered_multiset<Key, Hash, KeyEqual, Allocator>;
    using citer_t = typename container_t::const_iterator;
    using iter_t = typename container_t::iterator;
    using value_t = Key;

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

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
struct is_container<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>> : std::true_type
{
};
} // namespace rpc::adapters
