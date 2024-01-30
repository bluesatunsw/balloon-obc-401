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
template<typename T, typename I = std::uint32_t, typename P = std::span<std::byte>>
concept SendBus = requires(T& bus, const I& i, const P& p) {
	{ bus.Send(i, p) } -> void;
};

template<typename P = std::span<std::byte>>
class ListenCallback : public Callback<void, const P> {

};


template<typename T, typename P = std::span<std::byte>>
concept ListenBus = requires(T& bus, ListenCallback<P> cb) {
	typename T::ListenHandle;
	{ bus.Listen(cb) } -> T::ListenHandle;
};

template<typename P = std::span<std::byte>>
class RequestCallback : public Callback<void, const P> {

};

template<typename T, typename I = std::uint32_t, typename Req = std::span<std::byte>, typename Res = std::span<std::byte>>
concept RequestBus = requires(T& bus, const I& i, const Req& req, RequestCallback<Res> cb) {
	typename T::RequestHandle;
	{ bus.Request(i, req, cb) } -> T::RequestHandle;
};

      public:
        constexpr bool Complete() const { return m_complete; }
template<typename Req = std::span<std::byte>, typename Res = std::span<std::byte>>
class ProcessCallback : public Callback<Res, const Req> {

};
    using RequestHandle = Handle<RequestHandleData, Bus>;

    class ListenHandleData {
        friend Bus;

      private:
        TestCallback    m_test;
        ProcessCallback m_process;
    };

    using ListenHandle = Handle<ListenHandleData, Bus>;

    void Send(Packet message);

    template<typename T>
    void Send(const T& message) {
        Send(std::span(reinterpret_cast<std::byte*>(&message), sizeof(message))
        );
    }

    RequestHandle Request(Packet request, ProcessCallback callback);

    template<typename T>
    RequestHandle Request(const T& request, ProcessCallback callback) {
        return Request(
            std::span(reinterpret_cast<std::byte*>(&request), sizeof(request)),
            callback
        );
    }

    ListenHandle Listen(
        TestCallback test_callback, ProcessCallback process_callback
    );

  protected:
    virtual MessageId SendImpl(Packet message) = 0;

    inline void ProcessMessage(Packet message, MessageId id) {
        for (auto& handler : request_chain) {
            if (handler.m_id == id) {
                handler.m_callback(message);
                handler.m_complete = true;
                return;
            }
        }

        for (auto& handler : request_chain) {
            if (handler.m_test(message)) {
                handler.m_process(message);
                return;
            }
        }

        // TODO: log unhandled message
    }

  private:
    RequestHandle::Chain request_chain {};
    ListenHandle::Chain  listen_chain {};
template<typename T, typename Req = std::span<std::byte>, typename Res = std::span<std::byte>>
concept ProcessBus = requires(T& bus, ProcessCallback<Req, Res> cb) {
	typename T::ProcessHandle;
	{ bus.Process(cb) } -> T::ProcessHandle;
};
}  // namespace obc
