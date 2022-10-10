#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
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

    void deactivate()
    {
        m_active.store(false);
        m_cv.notify_all();
    }

    [[nodiscard]] bool is_active() const { return m_active.load(); }

    void push(const T& val)
    {
        if (!is_active())
        {
            return;
        }

        {
            auto lock = std::unique_lock{ m_mtx };
            m_mesg_queue.push(val);
        }

        m_cv.notify_one();
    }

    void push(T&& val)
    {
        if (!is_active())
        {
            return;
        }

        {
            auto lock = std::unique_lock{ m_mtx };
            m_mesg_queue.push(std::move(val));
        }

        m_cv.notify_one();
    }

    std::optional<T> pop()
    {
        if (is_active())
        {
            auto lock = std::unique_lock{ m_mtx };
            m_cv.wait(lock, [this] { return !is_active() || !m_mesg_queue.empty(); });

            if (is_active())
            {
                T val = m_mesg_queue.front();
                m_mesg_queue.pop();
                lock.unlock();
                m_cv.notify_one();
                return val;
            }
        }

        return std::nullopt;
    }

    size_t size() const { return m_mesg_queue.size(); }
    [[nodiscard]] bool empty() const { return m_mesg_queue.empty(); }

private:
    mutable std::atomic<bool> m_active{ false };
    mutable std::mutex m_mtx{};
    mutable std::condition_variable m_cv{};
    std::queue<T> m_mesg_queue{};
};
} //namespace rpc_hpp::tests
