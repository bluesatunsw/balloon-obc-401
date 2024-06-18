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

#include "obc/bus/types.hpp"

namespace obc::bus {
static_assert(Message<BasicMessage>);
static_assert(Buffer<std::span<std::byte>>);

template<typename T>
struct NullFilter {
    auto Filter(T /*arg*/) -> bool { return true; }
};

template<typename T>
constexpr NullFilter<T> kNullFilter {};

/**
 * @brief Convert a trivial struct into a readonly byte buffer.
 *
 * Can be used to facilitate trivial deserialization of C-style structs sent on
 * a bus.
 *
 * @warning For portability, the struct should be tightly packed (containing
 * explicit padding fields if required) with fixed sized integers.
 *
 * @tparam T type of the struct to translate.
 */
template<typename T>
    requires std::is_trivially_copyable_v<T>
auto StructAsBuffer(const T& s) -> std::span<const std::byte, sizeof(T)> {
    // This function is constrained to only operate on POD structs
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const std::byte*>(&s), sizeof(T)};
}

/**
 * @brief Convert a trivial struct into a read-write byte buffer.
 *
 * Can be used to facilitate trivial deserialization of C-style structs sent on
 * a bus.
 *
 * @warning For portability, the struct should be tightly packed (containing
 * explicit padding fields if required) with fixed sized integers.
 *
 * @tparam T type of the struct to translate.
 */
template<typename T>
    requires std::is_trivially_copyable_v<T>
auto StructAsBuffer(T& s) -> std::span<std::byte, sizeof(T)> {
    // This function is constrained to only operate on POD structs
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<std::byte*>(&s), sizeof(T)};
}

/**
 * @brief A helper class for implementing listening functionality.
 *
 * `FeedListeners` should be called for each incoming message. Typically
 * this should be invoked before feeding requesters and processors to
 * avoid them invalidating the data buffer which the message is potentially
 * stored in.
 *
 * @tparam M The type of message received.
 */
template<utils::MaybeError E = utils::Never, Message M = BasicMessage>
class ListenBusMixin {
    using ListenCallback = ipc::Callback<void, const std::expected<M, E>&>;
    using FilterCallback = ipc::Callback<bool, const std::expected<M, E>&>;

  public:
    using ListenHandle =
        utils::Handle<std::tuple<ListenCallback, FilterCallback>>;
    using ListenDispatchError = utils::Never;
    using ListenCallbackError = E;

    /**
     * @brief Adds a listener to be notified upon receiving a message.
     *
     * @param cb The listener callback to add.
     * @return An opaque handle that must be retained for the listener to remain
     * active.
     */
    auto Listen(
        ListenCallback&& cb,
        FilterCallback&& flt = OBC_CALLBACK_METHOD(
            (kNullFilter<const std::expected<M, E>&>), Filter
        )
    ) -> std::expected<ListenHandle, ListenDispatchError> {
        return ListenHandle(m_listeners, {std::move(cb), std::move(flt)});
    }

  protected:
    /**
     * @brief Forwards a message to all listeners.
     *
     * Should be called upon receiving any incoming message.
     *
     * @param msg The message to be forwarded.
     */
    auto FeedListeners(const std::expected<M, E>& msg) -> void {
        for (const auto& [cb, flt] : m_listeners)
            if (flt()) cb(msg);
    }

  private:
    utils::HandleChainRoot<ListenCallback> m_listeners {};
};
}  // namespace obc::bus
