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

#include <obc/mock/bus.hpp>

#include <gtest/gtest.h>

using namespace obc::bus::mock;

using obc::bus::StructAsBuffer;
using obc::bus::BasicMessage;
using obc::ipc::AsyncValue;

using testing::Eq;
using testing::ElementsAre;
using testing::StrictMock;

std::byte operator""_b(unsigned long long val) {
    return std::byte {static_cast<unsigned char>(val)};
}

class MockBusSend : public testing::Test {
  protected:
    StrictMock<MockSendBus<>> bus{};
};

TEST_F(MockBusSend, PushRaw) {
    std::array<std::byte, 4> msg_buf {0x42_b, 0xB0_b, 0xF2_b, 0x41_b};
    bus.Send({.address = 0x0A, .data = msg_buf});
    EXPECT_CALL(bus, Send(Eq(BasicMessage {.address = 0x0A, .data = msg_buf})));
}

struct __attribute__((packed)) DummyPushData {
    uint16_t foo;
    uint8_t bar;
};

TEST_F(MockBusSend, PushStruct) {
    DummyPushData data { .foo = 0x3132, .bar = 0xAE };
    bus.Send({.address = 0x4A, .data = StructAsBuffer(data)});
    std::array<std::byte, 3> buf{0x31_b, 0x32_b, 0xAe_b};
    EXPECT_CALL(bus, Send(Eq(BasicMessage {.address = 0x4A, .data = buf})));
}

class MockBusListen : public testing::Test {
  protected:
    MockListenBus<> bus{};
    AsyncValue<BasicMessage> listener{};
    std::array<std::byte, 4> msg_buf {0x42_b, 0xB0_b, 0xF2_b, 0x41_b};
};

TEST_F(MockBusListen, SingleValuePush) {
    bus.Listen(listener);
    bus.PushListenMessage({.address = 0xF0, .data = msg_buf});
    ASSERT_EQ(listener()->get().address, 0xF0);
    ASSERT_THAT(
        listener()->get().data,
        ElementsAre(0x42_b, 0xB0_b, 0xF2_b, 0x41_b)
    );
}

TEST_F(MockBusListen, RegisterAfterPush) {
    bus.PushListenMessage({.address = 0x00, .data = msg_buf});
    bus.Listen(listener);
    ASSERT_EQ(listener(), std::nullopt);
}

TEST_F(MockBusListen, MultipleListenersMultipleData) {
    bus.Listen(listener);
    bus.PushListenMessage({.address = 0xF0, .data = msg_buf});

    msg_buf[0] = 0x37_b;
    msg_buf[2] = 0xE1_b;
    bus.PushListenMessage({.address = 0x0A, .data = msg_buf});

    AsyncValue<BasicMessage> secondary_listener{};
    bus.Listen(secondary_listener);
    msg_buf[1] = 0xA4_b;
    msg_buf[3] = 0x69_b;
    bus.PushListenMessage({.address = 0x11, .data = msg_buf});

    ASSERT_EQ(listener()->get().address, 0xF0);
    ASSERT_THAT(
        listener()->get().data,
        ElementsAre(0x42_b, 0xB0_b, 0xF2_b, 0x41_b)
    );
    ASSERT_EQ(secondary_listener()->get().address, 0x11);
    ASSERT_THAT(
        listener()->get().data,
        ElementsAre(0x37_b, 0xA4_b, 0xE1_b, 0x69_b)
    );
}

class MockBusRequest : public testing::Test {
  protected:
};

TEST_F(MockBusRequest, SingleInFlight) {

}

TEST_F(MockBusRequest, MultipleInFlight) {
    
}
