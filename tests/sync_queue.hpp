///@file sync_queue.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of queue to pass messages between threads
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
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
            auto lock = std::unique_lock<std::mutex>{ m_mtx };
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
            auto lock = std::unique_lock<std::mutex>{ m_mtx };
            m_mesg_queue.push(std::move(val));
        }

        m_cv.notify_one();
    }

    std::optional<T> pop()
    {
        if (is_active())
        {
            auto lock = std::unique_lock<std::mutex>{ m_mtx };
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
