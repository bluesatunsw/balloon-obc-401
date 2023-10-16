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

/*
 * TODO: get move and delete to work properly mostly so that
 * bus request works
 */
template<typename T, typename F>
class Handle {
    friend F;

  private:
    class Chain {
        friend Handle;

      public:
        Handle* Top() const { return curr.load(); }

      private:
        Handle* ExchangeTop(Handle* new_top) { return curr.exchange(new_top); }

        std::atomic<Handle*> curr {nullptr};
    };

  public:
    Handle(const Handle& other) = delete;
    Handle(Handle&& other)      = delete;

    Handle& operator=(const Handle& other) = delete;
    Handle& operator=(Handle&& other)      = delete;

    ~Handle() = default;

    T& operator*() { return m_payload; }

    T* operator->() { return &m_payload; }

  private:
    Handle(T payload, Chain& chain) : m_payload(payload) {
        m_next = chain.ExchangeTop(this);
    };

    Handle* operator++() const { return m_next; }

    T             m_payload;
    Handle<T, F>* m_next;
};
