#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace rpc_hpp::tests
{
template<typename T>
class SyncQueue
{
public:
    void activate() { m_active.store(true); }
    void deactivate() { m_active.store(false); }
    [[nodiscard]] bool is_active() const { return m_active.load(); }

    void push(const T& val)
    {
        if (!m_active)
        {
            return;
        }

        auto lck = std::scoped_lock<std::mutex>{ m_mtx };
        m_mesg_queue.push(val);
    }

    void push(T&& val)
    {
        if (!m_active)
        {
            return;
        }

        auto lck = std::scoped_lock<std::mutex>{ m_mtx };
        m_mesg_queue.push(std::move(val));
    }

    std::optional<T> pop(const bool wait = true)
    {
        if ((!m_active) || ((!wait) && m_mesg_queue.empty()))
        {
            return std::nullopt;
        }

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
    std::atomic<bool> m_active{ false };
    mutable std::mutex m_mtx{};
    std::queue<T> m_mesg_queue{};
};
} //namespace rpc_hpp::tests
