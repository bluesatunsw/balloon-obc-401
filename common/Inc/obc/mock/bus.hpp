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

#include <gmock/gmock.h>

#include "obc/bus.hpp"

/**
 * @brief Mocks for communication busses.
 *
 * Use \ref TypeAmalgam to create a bus which satisfies multiple concepts.
 */
namespace obc::bus::mock {
/**
 * @brief Mock of \ref SendBus.
 *
 * @tparam M The type of message to send.
 */
template<Message M = BasicMessage>
class MockSendBus {
  public:
    MOCK_METHOD(void, Send, (const M& msg));
};

/**
 * @brief Mock of \ref ListenBus.
 *
 * @tparam M The type of message received.
 */
template<Message M = BasicMessage>
class MockListenBus : public ListenBusMixin<M> {
  public:
    /**
     * @brief Forwards a message to all listeners.
     *
     * @param msg The message to be forwarded.
     */
    void PushListenMessage(const M& msg) { FeedListeners(msg); }
};

/**
 * @brief Mock of \ref RequestBus.
 *
 * @tparam Req The type of message sent to request data.
 * @tparam Res The type of message received in response.
 * @tparam Flt The object used to determine if an incoming message is a response
 * to a given request.
 * @tparam BufUnit Smallest addressable unit of data within an incoming message.
 * @tparam Buf The type of the buffer which the result of a request is sent to.
 */
template<
    Message Req = BasicMessage, Message Res = BasicMessage,
    MessageFilter<Res> Flt     = ipc::Callback<bool, const Res&>,
    std::semiregular   BufUnit = std::byte,
    Buffer<BufUnit>    Buf     = std::span<BufUnit>>
class MockRequestBus : public RequestBusMixin<
                           MockRequestBus<Req, Res, Flt, BufUnit, Buf>, Req,
                           Res, Flt, BufUnit, Buf> {
  public:
    /**
     * @brief Forwards a message to all pending requests.
     *
     * @param res The response message to forward.
     */
    void PushRequestResponse(const Res& res) { FeedRequesters(res); }

    MOCK_METHOD(Flt, IssueRequest, (const Req& req));
};

/**
 * @brief Mock of \ref ProcessBus.
 *
 * @tparam Req The type of message received that requests data.
 * @tparam Res The type of message sent in response.
 */
template<Message Req = BasicMessage, Message Res = BasicMessage>
class MockProcessBus
    : public ProcessBusMixin<MockProcessBus<Req, Res>, Req, Res> {
  public:
    /**
     * @brief Forwards a message to processors.
     *
     * @param req The request message to forward.
     */
    void PushProcessResponse(const Res& res) { FeedProcessors(res); }

    MOCK_METHOD(void, IssueResponse, (const Req& req, const Res& res));
};
}  // namespace obc::bus::mock
