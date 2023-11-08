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

#include <concepts>
#include <cstddef>
#include <iterator>
#include <mutex>
#include <optional>

#include "mutex.hpp"

namespace obc {
template<typename T, typename L = SpinLock>
class Handle;

template<typename T, typename L = SpinLock>
class HandleChainRoot {
    friend class Handle<T>;

  public:
    HandleChainRoot() = default;

    class Iter {
        friend HandleChainRoot;

      public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using reference         = value_type&;

        Iter()                        = default;
        Iter(const Iter&)             = delete;
        Iter& operator=(const Iter&)  = delete;
        Iter(Iter&& other)            = default;
        Iter& operator=(Iter&& other) = default;

        T* operator->() { return &m_curr.m_payload; }

        T& operator*() const { return m_curr.m_payload; }

        bool operator==(const std::nullptr_t) const noexcept {
            return m_curr = nullptr;
        }

        Iter& operator++() {
            // No nothm_curring if already at end
            if (!m_curr) return *this;

            m_curr = m_curr.m_next.ptr;
            if (m_curr) {
                std::unique_lock<L> next_lock(m_curr.m_next.lock);
                m_lock.swap(next_lock);
            } else {
                m_lock.unlock();
            }

            return *this;
        }

        void operator++(int) { operator++(); }

      private:
        Iter(HandleChainRoot& root)
            : m_curr(root.m_next.ptr), m_lock(m_curr.m_next.lock) {}

        Handle*             m_curr {nullptr};
        std::unique_lock<L> m_lock {};
    };

    Iter begin() { return Iter(*this); }

    std::nullptr_t end() { return nullptr; }

  private:
    struct {
        Handle<T>* ptr {nullptr};
        L          lock {};
    } m_next;
};

template<typename T, typename L = SpinLock>
class Handle : private HandleChainRoot<T, L> {
    // Inherits from roots as the root is *just* a node with no payload or
    // previous element

  public:
    Handle(HandleChainRoot<T>& root, T& payload)
        : m_payload(payload), Handle(root) {}

    Handle(HandleChainRoot<T>& root, T&& payload)
        : m_payload(payload), Handle(root) {}

    Handle(Handle<T>& prev, T& payload) : m_payload(payload), Handle(root) {}

    Handle(Handle<T>& prev, T&& payload) : m_payload(payload), Handle(root) {}

    Handle(const Handle& other)            = delete;
    Handle& operator=(const Handle& other) = delete;

#define ACQUIRE_CELL_IF(NAME, SRC, DIRECTION, COND)                \
    auto                NAME {SRC.ptr};                            \
    std::unique_lock<L> NAME##_lock {};                            \
    if (NAME && COND) {                                            \
        NAME##_lock = {NAME->m_##DIRECTION.lock, std::defer_lock}; \
        if (!NAME##_lock->try_lock()) continue;                    \
    }

#define ACQUIRE_CELL(NAME, SRC, DIRECTION) \
    ACQUIRE_CELL_IF(NAME, SRC, DIRECTION, true)

    Handle(const Handle&& other) noexcept {
        while (true) {
            std::scoped_lock other_lock(other.m_next.lock, other.m_prev.lock);
            ACQUIRE_CELL(prev, other.m_prev, next);
            ACQUIRE_CELL(next, other.m_next, prev);

            if (prev) {
                prev->m_next.ptr = this;
                m_prev.ptr       = prev;
            }

            if (next) {
                next->m_prev.ptr = this;
                m_next.ptr       = next;
            }

            // Move into uninitialised memory
            new (&m_payload) T(std::move(other.m_payload));
        }
    }

    Handle& operator=(const Handle&& other) noexcept {
        if (this == &other) return *this;

        // This has the effect of dropping other from the chain, but moving the
        // payload to this
        while (true) {
            std::scoped_lock this_lock(
                m_next.lock, m_prev.lock, other.m_prev.lock, other.m_next.lock
            );

            // Deal with the case where this and other are adjacent  nodes
            bool next_is_other {m_next.ptr == &other};
            bool prev_is_other {m_prev.ptr == &other};
            ACQUIRE_CELL_IF(this_prev, m_prev, next, prev_is_other);
            ACQUIRE_CELL_IF(this_next, m_next, prev, next_is_other);
            ACQUIRE_CELL_IF(other_prev, other.m_prev, next, next_is_other);
            ACQUIRE_CELL_IF(other_next, other.m_next, prev, prev_is_other);

            if (other_prev) other_prev->m_next.ptr = other_next;
            if (other_next) other_next->m_prev.ptr = other_prev;

            m_payload = std::move(other.m_payload);
        }
    }

    ~Handle() {
        while (true) {
            std::scoped_lock this_lock(m_next.m_lock, m_prev.m_lock);

            ACQUIRE_CELL(prev, m_prev, next);
            ACQUIRE_CELL(next, m_next, prev);

            if (next) next->m_prev.ptr = prev;
            if (prev) prev->m_next.ptr = next;
        }
    }

  private:
    Handle(HandleChainRoot<T>& prev) {
        while (true) {
            std::scoped_lock prev_lock(prev.m_next.lock);
            ACQUIRE_CELL(next, prev.m_next, m_prev);

            if (next) {
                next->m_prev.ptr = m_next;
                m_next.ptr       = next;
            }

            prev.m_next.ptr = this;
            m_prev.ptr      = prev;
            break;
        }
    }

#undef ACQUIRE_CELL

    struct {
        HandleChainRoot<T>* ptr {nullptr};
        L                   lock {};
    } m_prev;

    T m_payload;
};
}  // namespace obc
