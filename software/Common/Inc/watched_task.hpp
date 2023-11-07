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

#include <functional>
#include <span>

#include "obctask.hpp"

namespace obc {
class WatchdogTask;

class WatchedTask : public virtual Task {
    friend WatchdogTask;

  protected:
    inline void Kick(bool fault = false) {
        m_fault |= fault;
        if (m_fault) return;

        if (m_timer < MinKickPeriod() || m_timer > MaxKickPeriod()) {
            m_fault = true;
            return;
        }

        m_timer = 0;
    }

    constexpr virtual units::quantised::Microseconds MinKickPeriod() const {
        return 0;
    }

    constexpr virtual units::quantised::Microseconds MaxKickPeriod() const {
        return 0;
    }

  private:
    inline bool Advance(units::quantised::Microseconds dt) {
        m_timer += dt;
        return m_fault;
    }

    units::quantised::Microseconds m_timer {0};
    bool                           m_fault {false};
};

constexpr std::uint32_t WatchdogStackSize = 512;

class WatchdogTask : public StackTask<WatchdogStackSize> {
  public:
    using Watchlist = std::span<std::reference_wrapper<WatchedTask>>;

    WatchdogTask(Watchlist watchlist) : m_watchlist {watchlist} {}

  protected:
    inline void Run() override {
        for (auto& task_wrapper : m_watchlist) {
            auto& task = task_wrapper.get();
            // TODO: actually get dt
            if (task.Advance(1000)) {
                // TODO: restart task
                // TODO: interface with system watch dog on fail of critical
            }
        }
    }

    constexpr const char* Name() const override { return "Watchdog"; }

    constexpr osPriority Priority() const override {
        return osPriorityHigh7;
    }

    constexpr units::quantised::Microseconds MinPeriod() const override {
        return 500;
    }

    constexpr units::quantised::Microseconds AvgPeriod() const override {
        return 1000;
    }

  private:
    Watchlist m_watchlist;
};
}  // namespace obc
