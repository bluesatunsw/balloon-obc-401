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

#include <bus.hpp>

#include <gmock/gmock.h>

namespace obc::bus::mock {
template<typename M = BasicMessage>
class MockSendBus {
  public:
    MOCK_METHOD(void, Send, (const M& msg), ());
};

template<typename M = BasicMessage>
class MockListenBus : public ListenBusMixin<M> {
  public:
    void PushListenMessage(const M& msg) { FeedListeners(msg); }
};

template<
    Message Req = BasicMessage, Message Res = BasicMessage,
    MessageFilter<Res> Flt     = ipc::Callback<bool, const Res&>,
    std::semiregular   BufUnit = std::byte,
    Buffer<BufUnit>    Buf     = std::span<BufUnit>>
class MockRequestBus : public RequestBusMixin<
                           MockRequestBus<Req, Res, Flt, BufUnit, Buf>, Req,
                           Res, Flt, BufUnit, Buf> {
  public:
    void PushRequestResponse(const M& res) { FeedRequesters(res); }

    MOCK_METHOD(Flt, IssueRequest, (const Req& req), ());
};

template<Message Req = BasicMessage, Message Res = BasicMessage>
class MockProcessBus : public ProcessBusMixin<D, Req, Res> {
  public:
    void PushProcessResponse(const M& res) { FeedProcessors(res); }

    MOCK_METHOD(void, IssueResponse, (const Req& req, const Res& res), ());
}
}  // namespace obc::bus::mock
