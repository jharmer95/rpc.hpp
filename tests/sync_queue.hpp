#pragma once

#include <cassert>
#include <mutex>
#include <queue>

namespace rpc_hpp::tests
{
template<typename T>
class SyncQueue
{
public:
    void push(const T& val)
    {
        auto lck = std::scoped_lock<std::mutex>{ m_mtx };
        m_mesg_queue.push(val);
    }

    void push(T&& val)
    {
        auto lck = std::scoped_lock<std::mutex>{ m_mtx };
        m_mesg_queue.push(std::move(val));
    }

    T pop(const bool wait = true)
    {
        assert(!wait || !m_mesg_queue.empty());

        while (wait && m_mesg_queue.empty())
        {
            // TODO: Use condition variable
        }

        auto lck = std::scoped_lock<std::mutex>{ m_mtx };
        T val = m_mesg_queue.front();
        m_mesg_queue.pop();
        return val;
    }

    size_t size() const { return m_mesg_queue.size(); }
    [[nodiscard]] bool empty() const { return m_mesg_queue.empty(); }

private:
    mutable std::mutex m_mtx;
    std::queue<T> m_mesg_queue{};
};
} //namespace rpc_hpp::tests
