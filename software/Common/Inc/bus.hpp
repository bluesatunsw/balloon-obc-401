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
#include <optional>
#include <span>

#include "callback.hpp"
#include "handle.hpp"

namespace obc {
// Send
template<
    typename T, typename I = std::uint32_t, typename P = std::span<std::byte>>
concept SendBus =
    requires(T& bus, const I& i, const P& p) {
        typename T::Identifier;
        typename T::Packet;

        { bus.Send(i, p) } -> std::same_as<void>;
    } && std::convertible_to<I, typename T::Identifier> &&
    std::convertible_to<P, typename T::Packet>;

// Listen
template<typename I = std::uint32_t, typename P = std::span<std::byte>>
using ListenCallback = Callback<void, const I, const P>;

template<
    typename T, typename I = std::uint32_t, typename P = std::span<std::byte>>
concept ListenBus =
    requires(T& bus, ListenCallback<P> cb) {
        typename T::Identifier;
        typename T::Packet;
        typename T::ListenHandle;

        { bus.Listen(cb) } -> std::same_as<typename T::ListenHandle>;
    } && std::convertible_to<I, typename T::Identifier> &&
    std::convertible_to<P, typename T::Packet>;

template<
    typename B, typename I = std::uint32_t, typename P = std::span<std::byte>>
class ListenBusMixin {
    using ThisListenCallback = ListenCallback<I, P>;

  public:
    using ListenHandle = Handle<ThisListenCallback>;

    ListenHandle Listen(ThisListenCallback& cb) {
        return ListenHandle(m_listen_callbacks, cb);
    }

  protected:
    void FeedListeners(I i, P p) {
        for (auto& callback : m_listen_callbacks) callback(i, p);
    }

  private:
    HandleChainRoot<ThisListenCallback> m_listen_callbacks;
};

// Request
template<typename I = std::uint32_t, typename P = std::span<std::byte>>
using RequestCallback = Callback<void, const I, const P>;

template<
    typename T, typename I = std::uint32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
concept RequestBus =
    requires(T& bus, const I& i, const Req& req, RequestCallback<Res> cb) {
        typename T::Identifier;
        typename T::RequestPacket;
        typename T::ResponsePacket;
        typename T::RequestHandle;

        { bus.Request(i, req, cb) } -> std::same_as<typename T::RequestHandle>;
    } && std::convertible_to<I, typename T::Identifier> &&
    std::convertible_to<Req, typename T::RequestPacket> &&
    std::convertible_to<Res, typename T::ResponsePacket>;

template<
    typename B, typename I = std::uint32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
class RequestBusMixin {
    using ThisRequestCallback = RequestCallback<I, Res>;

    struct RequestHandleData {
        B::RequestReponseFilter filter;
        ThisRequestCallback     callback;
    };

  public:
    using RequestHandle = Handle<RequestHandleData>;

    RequestHandle Request(I i, Req req, ThisRequestCallback& cb) {
        return RequestHandle(
            m_request_callbacks,
            {
                .filter   = static_cast<B*>(this)->SendRequest(i, req),
                .callback = cb,
            }
        );
    }

  protected:
    void FeedRequesters(I i, Res p) {
        for (auto& callback : m_request_callbacks)
            if (callback.filter(i, p)) callback.callback(i, p);
    }

  private:
    HandleChainRoot<RequestHandleData> m_request_callbacks;
};

// Process
template<
    typename I = std::uint32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
using ProcessCallback = Callback<std::optional<Res>, const I, const Req>;

template<
    typename T, typename I = std::uint32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
concept ProcessBus =
    requires(T& bus, ProcessCallback<Req, Res> cb) {
        typename T::Identifier;
        typename T::RequestPacket;
        typename T::ResponsePacket;
        typename T::ProcessHandle;

        { bus.Process(cb) } -> std::same_as<typename T::ProcessHandle>;
    } && std::convertible_to<I, typename T::Identifier> &&
    std::convertible_to<Req, typename T::RequestPacket> &&
    std::convertible_to<Res, typename T::ResponsePacket>;

template<
    typename B, typename I = std::uint32_t, typename Req = std::span<std::byte>,
    typename Res = std::span<std::byte>>
class ProcessBusMixin {
    using ThisProcessCallback = ProcessCallback<I, Req, Res>;

  public:
    using ProcessHandle = Handle<ThisProcessCallback>;

    ProcessHandle Process(I i, Req req, ThisProcessCallback& cb) {
        return ProcessHandle(m_processor_callbacks, cb);
    }

  protected:
    void FeedProcessors(I i, Req p) {
        for (auto& callback : m_processor_callbacks)
            if (auto res = callback(i, p))
                static_cast<B*>(this)->SendResponse(i, *res);
    }

  private:
    HandleChainRoot<ThisProcessCallback> m_processor_callbacks;
};
}  // namespace obc
