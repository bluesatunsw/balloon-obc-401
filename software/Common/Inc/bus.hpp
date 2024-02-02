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
#include <cassert>
#include <cstddef>
#include <span>

#include "callback.hpp"
#include "handle.hpp"

namespace obc {
// Send
template<
    typename T, typename I = std::uint32_t, typename P = std::span<std::byte>>
concept SendBus = requires(T& bus, const I& i, const P& p) {
    typename T::Identifier;
    typename T::Packet;

    { bus.Send(i, p) } -> void;
} && std::convertible_to<I, T::Identifier> && std::convertible_to<P, T::Packet>;

// Listen
template<typename I = std::uint32_t, ypename P = std::span<std::byte>>
using ListenCallback = Callback<void, const I, const P>;

template<
    typename T, typename I = std::uint32_t, typename P = std::span<std::byte>>
concept ListenBus = requires(T& bus, ListenCallback<P> cb) {
    typename T::Identifier;
    typename T::Packet;
    typename T::ListenHandle;

    { bus.Listen(cb) } -> T::ListenHandle;
} && std::convertible_to<I, T::Identifier> && std::convertible_to<P, T::Packet>;

// Request
template<typename P = std::span<std::byte>>
using RequestCallback = Callback<void, const P>;

template<
    typename T, typename I = std::uint32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
concept RequestBus =
    requires(T& bus, const I& i, const Req& req, RequestCallback<Res> cb) {
        typename T::Identifier;
        typename T::RequestPacket;
        typename T::ResponsePacket;
        typename T::RequestHandle;

        { bus.Request(i, req, cb) } -> T::RequestHandle;
    } && std::convertible_to<I, T::Identifier> &&
    std::convertible_to<Req, T::RequestPacket> &&
    std::convertible_to<Res, T::ResponsePacket>;

// Process
template<
    typename I = std::unit32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
using RequestCallback = Callback<Res, const I, const Req>;

template<
    typename T, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
concept ProcessBus =
    requires(T& bus, ProcessCallback<Req, Res> cb) {
        typename T::Identifier;
        typename T::RequestPacket;
        typename T::ResponsePacket;
        typename T::ProcessHandle;

        { bus.Process(cb) } -> T::ProcessHandle;
    } && std::convertible_to<I, T::Identifier> &&
    std::convertible_to<Req, T::RequestPacket> &&
    std::convertible_to<Res, T::ResponsePacket>;
}  // namespace obc
