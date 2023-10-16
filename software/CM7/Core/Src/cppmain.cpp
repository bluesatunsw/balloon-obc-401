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

#include "cppmain.hpp"

#include "bus.hpp"
#include "watched_task.hpp"

class DummyBus : public obc::Bus,
                 public obc::StackTask<>,
                 public obc::WatchedTask {
    void Run() override {
        // Recieve message
        std::array<std::byte, 12> msg {
            std::byte {'H'}, std::byte {'e'}, std::byte {'l'}, std::byte {'l'},
            std::byte {'o'}, std::byte {' '}, std::byte {'W'}, std::byte {'o'},
            std::byte {'r'}, std::byte {'l'}, std::byte {'d'}, std::byte {'!'}};

        ProcessMessage(msg, 42);
    }

    MessageId SendImpl(Packet message) override {
        // TODO: do nothing
    }

    constexpr const char* Name() const override { return "Dummy Bus"; }
};

struct StaticData {
    struct {
        DummyBus dummy;
    } busses;

    struct {
    } devices;

    struct {
    } subsystems;

    std::array<std::reference_wrapper<obc::WatchedTask>, 1> watchlist {
        busses.dummy};
    obc::WatchdogTask watchdog {watchlist};
};

extern "C" {
// Add on an extra 7 bytes to ensure alignment
const unsigned long static_buf_size = sizeof(StaticData) + 7;

void CppMain(void* static_buf) {
    auto& static_data = *(new (static_buf) StaticData);
}
}
