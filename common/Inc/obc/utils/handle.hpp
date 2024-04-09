/* USER CODE BEGIN Header */
/*
 * 401 Ballon OBC
 * Copyright (C) 2024 Bluesat and contributors.
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

#include "ipc/mutex.hpp"

namespace obc::utils {
template<typename T, typename L = ipc::SpinLock>
class Handle;

/**
 * @brief Root node in a linked list of handles.
 *
 * Provides forward iteration through the handles and acts as a range. It is
 * safe to allow this object to go out of scope even if it contains nodes.
 *
 * @tparam T Data stored in each node.
 * @tparam L Type of lock used in each node.
 */
template<typename T, typename L = ipc::SpinLock>
class HandleChainRoot {
  public:
    class Iter;
    friend class Iter;

    /**
     * @brief Initializes a handle chain with no elements.
     */
    HandleChainRoot() = default;

    /**
     * @brief Input iterator for traversing the handle chain.
     *
     * The lock on the pointer to the next node in the chain will always be
     * held, unless the iterator is the null sentinal. This is to prevent the
     * current node from being invalidated.
     */
    class Iter {
      private:
        /* Lock member must proceed data member to ensure that the lock is
         * acquired before attempts to load data are made.
         */
        std::unique_lock<L> m_lock {};
        Handle<T, L>*       m_curr {nullptr};

        friend HandleChainRoot;

      public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using reference         = value_type&;

        Iter() = default;

        Iter(const Iter&)            = delete;
        Iter& operator=(const Iter&) = delete;

        Iter(Iter&& other)            = default;
        Iter& operator=(Iter&& other) = default;

        /**
         * @brief Get the payload of the current node.
         *
         * @return Pointer to the payload.
         */
        T* operator->() { return &m_curr->m_payload; }

        /**
         * @brief Get the payload of the current node.
         *
         * @return Reference to the payload.
         */
        T& operator*() const { return m_curr->m_payload; }

        /**
         * @brief Compares the iterator to the sentinal value to check if at the
         * end of iteration.
         *
         * @return True if the iterator is at the end, false otherwise.
         */
        bool operator==(const std::nullptr_t) const noexcept {
            return m_curr == nullptr;
        }

        /**
         * @brief Shift to the next node in the handle chain.
         *
         * @return Reference to the shifted iterator.
         */
        Iter& operator++() {
            m_curr = m_curr->m_next.ptr;
            if (m_curr) {
                std::unique_lock<L> next_lock(m_curr->m_next.lock);
                m_lock.swap(next_lock);
            } else {
                m_lock.unlock();
            }

            return *this;
        }

        /**
         * @brief Shifts the iterator to the next element in the chain.
         */
        void operator++(int) { operator++(); }

      private:
        /**
         * @brief Constructs an iterator pointing to the first element of the
         * handle chain.
         *
         * @param root Reference to the chain root.
         */
        Iter(HandleChainRoot& root)
            : m_lock(m_curr->m_next.lock), m_curr(root.m_next.ptr) {
            // After the pointer to the first real node has safely been
            // acquired, lock the pointer to the next node.
            m_lock.swap(m_curr->m_next.lock);
        }
    };

    /**
     * @brief Creates an iterator pointing to the beginning of the handle chain.
     *
     * @return An iterator to the first element of the chain.
     */
    Iter begin() { return Iter(*this); }

    /**
     * @brief Gets the end of iteration sentinal for the chain.
     *
     * @return nullptr, as this is used as the universal sentinal.
     */
    std::nullptr_t end() { return nullptr; }

  public:
    /**
     * @brief Structure to hold the pointer to the next node and a lock for
     * synchronization.
     */
    struct {
        Handle<T, L>* ptr {nullptr};
        L             lock {};
    } m_next;
};

/**
 * @brief Represents an opaque object that must be held for the duration
 * specified by the function that returned it.
 *
 * The publisher-subscriber pattern, common for message passing, allows an
 * arbitrary number of device drivers to listen to messages arriving on a bus.
 * This pattern is typically implemented as a list of subscribers owned by the
 * publisher; however, in embedded environments where dynamic memory allocation
 * is undesirable or forbidden, a method to create such a list without dynamic
 * memory is required.
 *
 * A handle acts as a node in a doubly linked list. Rather than creating nodes
 * on the heap, their storage becomes the responsibility of whoever inserts a
 * node into the list.
 *
 * For example, if an object registers a callback and receives a handle it
 * must store it. Failing to store the handle results in the immediate
 * deregistration of the callback, as upon destruction of the handle, the node
 * is removed from the linked list.
 *
 * Operations on handles are thread-safe, implemented using standard
 * algorithms with locking.
 *
 * @tparam T Data stored in the node.
 * @tparam L Type of lock used.
 */
template<typename T, typename L>
class Handle : private HandleChainRoot<T, L> {
    /*
     * Inherits from roots as the root is *just* a node with no payload or
     * previous element. This allows the pointer to the previous node to be
     * generic.
     */

/**
 * @brief Acquires the lock associated with an adjacent node if it exists and
 * a certain condition is met.
 *
 * If the pointer is not null, this implies that an adjacent node exists.
 * To safely perform operations that invalidate pointers to this node,
 * the pointer in the other node must be locked.
 *
 * If the lock exists and can't be acquired immediately, the outer loop is
 * retried (`continue` is invoked).
 *
 * A macro is utilized here for conciseness and clarity, which would be
 * cumbersome with regular C++ constructs.
 *
 * @param NAME Name for the pointer to the other node.
 * @param SRC Source pointer to the other node.
 * @param DIRECTION Direction from the other node to this node (either prev or
 * next).
 * @param COND Condition that must be true before acquiring the lock.
 */
#define ACQUIRE_CELL_IF(NAME, SRC, DIRECTION, COND)                \
    auto                NAME {SRC.ptr};                            \
    std::unique_lock<L> NAME##_lock {};                            \
    if (NAME && COND) {                                            \
        NAME##_lock = {NAME->m_##DIRECTION.lock, std::defer_lock}; \
        if (!NAME##_lock.try_lock()) continue;                     \
    }

/**
 * @brief Acquires the lock associated with an adjacent node if it exists.
 *
 * See \ref ACQUIRE_CELL_IF for more information.
 */
#define ACQUIRE_CELL(NAME, SRC, DIRECTION) \
    ACQUIRE_CELL_IF(NAME, SRC, DIRECTION, true)

  public:
    /**
     * @brief Creates a new handle and inserts it immediately after the root
     * node.
     *
     * The node is inserted directly after the root node. Note that the root
     * node type does not imply it is actually the root of the list, as the
     * regular handle type is derived from the root node type.
     *
     * @param root The node after which this new node will be inserted.
     * @param payload The data to be stored in the node.
     */
    Handle(HandleChainRoot<T, L>& root, T&& payload) : m_payload(payload) {
        while (true) {
            std::scoped_lock this_lock(this->m_prev.lock, this->m_next.lock);
            ACQUIRE_CELL(next, root.m_next, prev);

            if (next) {
                next->m_prev.ptr = this;
                this->m_next.ptr = next;
            }

            root.m_next.ptr  = this;
            this->m_prev.ptr = &root;
            break;
        }
    }

    Handle(const Handle& other)            = delete;
    Handle& operator=(const Handle& other) = delete;

    /**
     * @brief Moves the handle to a new location, updating pointers in adjacent
     * nodes.
     *
     * Adjacent nodes are updated to point to the new location of the handle.
     */
    Handle(Handle&& other) noexcept {
        while (true) {
            std::scoped_lock other_lock(other.m_next.lock, other.m_prev.lock);
            ACQUIRE_CELL(prev, other.m_prev, next);
            ACQUIRE_CELL(next, other.m_next, prev);

            if (prev) {
                prev->m_next.ptr = this;
                this->m_prev.ptr = prev;
            }

            if (next) {
                next->m_prev.ptr = this;
                this->m_next.ptr = next;
            }

            // Move into uninitialized memory
            new (&m_payload) T(std::move(other.m_payload));
            break;
        }
    }

    /**
     * @brief Moves another existing handle into this location, replacing its
     * contents.
     *
     * The other handle is removed from the chain, and its contents are moved
     * into this one. Adjacent nodes to the other node are updated to bypass it.
     */
    Handle& operator=(Handle&& other) noexcept {
        if (this == &other) return *this;

        while (true) {
            std::scoped_lock this_lock(
                this->m_next.lock, this->m_prev.lock, other.m_prev.lock,
                other.m_next.lock
            );

            // Handles cases where this and other are adjacent
            bool next_is_other {this->m_next.ptr == &other};
            bool prev_is_other {this->m_prev.ptr == &other};
            ACQUIRE_CELL_IF(this_prev, this->m_prev, next, !prev_is_other);
            ACQUIRE_CELL_IF(this_next, this->m_next, prev, !next_is_other);
            ACQUIRE_CELL_IF(other_prev, other.m_prev, next, !next_is_other);
            ACQUIRE_CELL_IF(other_next, other.m_next, prev, !prev_is_other);

            if (other_prev) other_prev->m_next.ptr = other_next;
            if (other_next) other_next->m_prev.ptr = other_prev;

            m_payload = std::move(other.m_payload);
            break;
        }
    }

    /**
     * @brief Destructs the handle, removing it from the chain.
     *
     * Adjacent nodes are updated to bypass this node, effectively removing it
     * from the list.
     */
    ~Handle() {
        while (true) {
            std::scoped_lock this_lock(this->m_next.lock, this->m_prev.lock);

            ACQUIRE_CELL(prev, this->m_prev, next);
            ACQUIRE_CELL(next, this->m_next, prev);

            if (next) next->m_prev.ptr = prev;
            if (prev) prev->m_next.ptr = next;
            break;
        }
    }

  private:
// Ensuring macro hygiene
#undef ACQUIRE_CELL_IF
#undef ACQUIRE_CELL

    struct {
        HandleChainRoot<T, L>* ptr {nullptr};
        L                      lock {};
    } m_prev;

    T m_payload;
};
}  // namespace obc::utils
