/* USER CODE BEGIN Header */
/*
 * 401 Ballon OBC
 * Copyright (C) 2023 BLUEsat and contributors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
/* USER CODE END Header */

#pragma once

#include <atomic>
#include <concepts>
#include <mutex>

#include "mutex.hpp"

namespace obc {
template<typename T, typename F>
class Handle {
    friend F;

  public:
    class It;

    Handle(Handle&& other) {
        auto lock {other.Lock()};

        m_payload    = std::move(other.m_payload);
        m_prev       = other.m_prev;
        m_next       = other.m_next;
        other.m_prev = nullptr;
        other.m_next = nullptr;

        m_prev->m_next = this;
        m_next->m_prev = this;
    }

    Handle& operator=(Handle&& other) {
        if (this == &other) return *this;
        auto lock {Lock()};
        other.DropThen([this]() { m_payload = std::move(other.m_payload); });
    }

    friend void swap(Handle& lhs, Handle& rhs) {
        std::scoped_lock lock {
            lhs.m_prev_lock, lhs.m_next_lock, rhs.m_prev_lock, rhs.m_next_lock};
        using std::swap;
        swap(lhs.m_payload, rhs.m_payload);
    }

    ~Handle() {
        DropThen([]() {});
    }

  private:
    class Chain;

    Handle(T&& payload, Chain& chain) : m_payload(payload), m_prev(nullptr) {
        std::scoped_lock chain_lock {chain.m_lock};
        m_next = chain.m_head;
        if (chain.m_head) {
            std::scoped_lock head_lock {chain.m_head->m_prev_lock};
            chain.m_head->m_prev = this;
        }
    }

    template<typename F>
    void DropThen(F f, Handle* skip_locking = nullptr) {
        while (true) {
            auto lock {Lock()};

            std::unique_lock prev_lock {m_prev->m_next_lock, std::try_to_lock};
            // Abort and retry to if the adjacent lock cannot be acquired
            if (m_prev != skip_locking && !prev_lock.owns_lock()) continue;

            std::unique_lock next_lock {m_next->m_prev_lock, std::try_to_lock};
            // Abort and retry to if the adjacent lock cannot be acquired
            if (m_next != skip_locking && !next_lock.owns_lock()) continue;

            m_prev->m_next = m_next;
            m_next->m_prev = m_prev;
            m_prev         = nullptr;
            m_next         = nullptr;
            f();
            break;
        }
    }

    auto Lock() { return std::scoped_lock {m_prev_lock, m_next_lock}; }

    volatile T       m_payload;
    volatile Handle* m_prev;
    volatile Handle* m_next;
    SpinLock         m_prev_lock {};
    SpinLock         m_next_lock {};
};

template<typename T, typename F>
class Handle<T, F>::It {
  public:
    friend auto operator<=>(const It&, const It&) = default;

    T& operator*() { return **m_ptr; }

    It operator++() {
        m_ptr->m_next_lock();
        return {*m_ptr->m_next};
    }

  private:
    It() {}

    It(Handle* ptr) : m_ptr(ptr) {}

    Handle* m_ptr {nullptr};
};

template<typename T, typename F>
class Handle<T, F>::Chain {
    friend Handle;

  public:
    Handle::It begin() const {
        std::scoped_lock {m_lock};
        return { m_head }
    };

    Handle::It end() const {return {m_head}};

  private:
    volatile Handle* m_head {nullptr};
    SpinLock         m_lock {};
}
}  // namespace obc
