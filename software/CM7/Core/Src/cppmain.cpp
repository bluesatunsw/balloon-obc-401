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

#include <cstddef>

#include "bus.hpp"
#include "callback.hpp"
#include "watched_task.hpp"

class DummyBus : public obc::StackTask<>,
                 public obc::WatchedTask,
                 public obc::ListenBusMixin<DummyBus> {
    using Identifier = std::uint32_t;
    using Packet     = std::span<std::byte>;

    void Run() override {
        // Recieve message
        std::array<std::byte, 12> msg {
            std::byte {'H'}, std::byte {'e'}, std::byte {'l'}, std::byte {'l'},
            std::byte {'o'}, std::byte {' '}, std::byte {'W'}, std::byte {'o'},
            std::byte {'r'}, std::byte {'l'}, std::byte {'d'}, std::byte {'!'}};
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

    // obc::WatchdogTask watchdog {watchlist};
};

alignas(StaticData) std::byte static_buf[sizeof(StaticData)];
extern "C" {
void CppMain() { auto& static_data = *(new (static_buf) StaticData); }
}
