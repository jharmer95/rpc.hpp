#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <optional>
#include <queue>
#include <thread>
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
        if (!m_active.load())
        {
            return;
        }

        m_mesg_queue.push(val);
        m_in_use.store(true);
    }

    void push(T&& val)
    {
        if (!m_active.load())
        {
            return;
        }

        m_mesg_queue.push(std::move(val));
        m_in_use.store(true);
    }

    std::optional<T> pop()
    {
        while (m_active.load() && !m_in_use.load())
        {
            // TODO: Use condition variable
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }

        if (!m_active.load())
        {
            return std::nullopt;
        }

        T val = m_mesg_queue.front();
        m_mesg_queue.pop();

        if (empty())
        {
            m_in_use.store(false);
        }

        return val;
    }

    size_t size() const { return m_mesg_queue.size(); }
    [[nodiscard]] bool empty() const { return m_mesg_queue.empty(); }

private:
    std::atomic<bool> m_active{ false };
    std::atomic<bool> m_in_use{ false };
    std::queue<T> m_mesg_queue{};
};
} //namespace rpc_hpp::tests
