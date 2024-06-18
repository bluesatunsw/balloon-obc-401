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
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <units/time.h>

#include "obc/ipc/callback.hpp"
#include "obc/utils/error.hpp"
#include "obc/utils/handle.hpp"
#include "obc/utils/meta.hpp"

/**
 * @brief Defines a common interface for communication buses.
 *
 * The purpose of a bus is to abstract the network stack up to, but not
 * including, the application layer. Operations are defined in terms of
 * concepts, allowing implementers to choose a subset of supported operations
 * and users to select a subset of required operations. Mixins are provided to
 * aid in implementing some bus operations, though their use is optional.
 *
 * @warning Given its rarity in the majority of embedded communication
 * standards, and due to its limited utility and the complexity of
 * implementation, socket-style interfaces are not supported.
 *
 * Operations involve sending and/or receiving messages. The specific type of
 * message is a template parameter, allowing for additional functionality or
 * metadata to be provided. This approach is preferred over creating additional
 * functions to ensure consistency.
 */
namespace obc::bus {
/**
 * @brief Represents a message sent or received by a bus.
 *
 * Must include an address and data (payload).
 *
 * @warning Addresses do not necessarily correspond to a single physical device
 * (or any device at all); buses may define implementation-specific addresses
 * that additional provide functionalities, including but not limited to:
 * multicasting, loopback, and voiding.
 */
template<typename T>
concept Message = requires(T& msg) {
    typename T::Address;
    typename T::Data;

    { msg.address } -> std::same_as<typename T::Address&>;
    { msg.data } -> std::same_as<typename T::Data&>;
} && std::regular<typename T::Address> && std::semiregular<typename T::Data>;

/**
 * @brief Represents a generic message type.
 *
 * If there are no special requirements, this should be used as the type for all
 * bus operations. Where feasible, more specialized message types should be
 * convertible to and from this basic message type, allowing users to ignore
 * more advanced functionality if not required.
 */
struct BasicMessage {
    using Address = std::uint32_t;
    using Data    = std::span<std::byte>;

    Address address {};
    Data    data {};
};

/**
 * @brief A generic buffer type which behaves like std::span.
 *
 * Used for storing the the payload of packets received in operations where the
 * size of the message may be unbound (from the perspective of the bus).
 *
 * @tparam V Fundamental unit of data stored in the buffer.
 */
template<typename T, typename V = std::byte>
concept Buffer = std::semiregular<V> && std::ranges::output_range<T, V> &&
                 std::ranges::sized_range<T>;

/**
 * @brief An object which behaves (and can be used like) an opaque unique
 * handle.
 */
template<typename T>
concept HandleLike = std::movable<T> && std::destructible<T>;

/**
 * @brief Represents a callable that returns a bool in response to a packet.
 *
 * Used to match an arbitrary criteria on a packet to filter them.
 */
template<
    typename T, typename R, typename E = utils::Never,
    typename M = BasicMessage>
concept MessageCallback = requires(T& cb, const std::expected<M, E>& msg) {
    { cb(msg) } -> std::convertible_to<R>;
} && HandleLike<T> && utils::MaybeError<E> && Message<M>;

template<typename R, typename E, typename M>
auto MessageCallbackProvider() {
    return std::declval<
        ipc::Callback<R, const std::expected<M, E>&>::DummyProvider>();
}

/**
 * @brief Represents a bus that can send packets of data to an address.
 *
 * Includes a function `Send(msg, cb)` for sending a message on the bus.
 * Upon completition of the send, the callback `cb` is invoked.
 *
 * @tparam M The type of message to send.
 */
template<typename T, typename M = BasicMessage>
concept SendBus =
    requires(T& bus, const M& msg) {
        typename T::SendHandle;
        typename T::SendDispatchError;
        typename T::SendCallbackError;

        {
            bus.Send(
                msg,
                MessageCallbackProvider<void, typename T::SendCallbackError, M>(
                )
            )
        } -> std::same_as<std::expected<
            typename T::SendHandle, typename T::SendDispatchError>>;
    } && HandleLike<typename T::SendHandle> &&
    utils::MaybeError<typename T::SendDispatchError> &&
    utils::MaybeError<typename T::SendCallbackError> && Message<M>;

/**
 * @brief Represents a bus that can listen to all received messages.
 *
 * Includes a function `Listen(cb)` for registering a listener callback that is
 * invoked for every incoming message.
 *
 * A filter can be specified so that the callback is only invoked if the filter
 * matches the method. In the most basic form, a filter is an arbitary pure
 * predicate function, implementation defined optimisation may be applied to
 * avoid actually invoking the filter function.
 *
 * @tparam M The type of message received.
 */
template<typename T, typename M = BasicMessage>
concept ListenBus =
    requires(T& bus) {
        typename T::ListenHandle;
        typename T::ListenDispatchError;
        typename T::ListenCallbackError;

        {
            bus.Listen(MessageCallbackProvider<
                       void, typename T::ListenCallbackError, M>())
        } -> std::same_as<std::expected<
            typename T::ListenHandle, typename T::ListenDispatchError>>;
        {
            bus.Listen(
                MessageCallbackProvider<
                    void, typename T::ListenCallbackError, M>(),
                MessageCallbackProvider<
                    bool, typename T::ListenCallbackError, M>()
            )
        } -> std::same_as<std::expected<
            typename T::ListenHandle, typename T::ListenDispatchError>>;
    } && HandleLike<typename T::ListenHandle> &&
    utils::MaybeError<typename T::ListenDispatchError> &&
    utils::MaybeError<typename T::ListenCallbackError> && Message<M>;

/**
 * @brief Represents a bus capable of requesting data from a remote device.
 *
 * Includes a function `Request(req, cb, buf)` for sending a request and
 * registering a callback that is invoked when a response arrives. Multiple
 * responses per request are allowed.
 *
 * @tparam Req The type of message sent to request data.
 * @tparam Res The type of message received in response.
 * @tparam BufUnit Smallest addressable unit of data within an incoming message.
 * @tparam Buf The type of the buffer which the result of a request is sent to.
 */
template<
    typename T, typename Req = BasicMessage, typename Res = BasicMessage,
    typename BufUnit = std::byte, typename Buf = std::span<BufUnit>>
concept RequestBus =
    requires(T& bus, const Req& req, Buf& buf) {
        typename T::RequestHandle;
        typename T::RequestDispatchError;
        typename T::RequestCallbackError;

        {
            bus.Request(
                req,
                MessageCallbackProvider<void, T::RequestCallbackError, Res>(),
                buf
            )
        } -> std::same_as<std::expected<
            typename T::RequestHandle, typename T::RequestDispatchError>>;
    } && HandleLike<typename T::RequestHandle> &&
    utils::MaybeError<typename T::RequestDispatchError> &&
    utils::MaybeError<typename T::RequestCallbackError> && Message<Req> &&
    Message<Res> && Buffer<Buf, BufUnit>;
}  // namespace obc::bus
