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

#include "mutex.hpp"

namespace obc {
template<typename T, typename F>
class Handle {
    friend F;

  private:
    class Chain {
        friend Handle;

      private:
        class HandleIt {
          public:
            HandleIt(Handle* ptr) : m_ptr(ptr) {}

            friend auto operator<=>(const HandleIt&, const HandleIt&) = default;

            T& operator*() {return **m_ptr}
            HandleIt operator++() {return {*m_ptr->m_next};}

          private:
            Handle* m_ptr;
        };

      public:
        HandleIt begin() const { return {m_head}; }
        constexpr HandleIt end() const { return {nullptr}; }

      private:
        volatile Handle* m_head {nullptr};
        SpinLock         m_lock {};
    };

  public:
    Handle(const Handle& other) = delete;

    Handle(Handle&& other)
        : m_payload(other.m_payload), m_prev(other.m_prev),
          m_next(other.m_next), m_chain(other.m_chain) {
        std::scoped_lock lock {m_chain->m_lock};
        m_prev->m_next = this;
        m_next->m_prev = this;

        other.m_prev = nullptr;
        other.m_next = nullptr;
    }

    Handle& operator=(const Handle& other) = delete;

    Handle& operator=(Handle&& other) {
        if (this == &other) return *this;

        m_payload = other.m_payload;
        m_prev    = other.m_prev;
        m_next    = other.m_next;
        m_chain   = other.m_chain;

        std::scoped_lock lock {m_chain->m_lock};
        m_prev->m_next = this;
        m_next->m_prev = this;

        other.m_prev = nullptr;
        other.m_next = nullptr;
    }

    ~Handle() {
        std::scoped_lock lock {m_chain->m_lock};
        m_prev->m_next = m_next;
        m_next->m_prev = m_prev;
    }

    T& operator*() { return m_payload; }
    T* operator->() { return &m_payload; }

  private:
    Handle(T payload, Chain& chain) : m_payload(payload), m_chain(&chain) {
        std::scoped_lock lock {m_chain->m_lock};
        m_next = m_chain->m_head;
        if (!m_chain->m_head) m_chain->m_head->m_next = this;
        m_chain->m_head = this;
    };

    T                m_payload;
    volatile Handle* m_prev {nullptr};
    volatile Handle* m_next {nullptr};
    Chain*           m_chain {};
};
}  // namespace obc
