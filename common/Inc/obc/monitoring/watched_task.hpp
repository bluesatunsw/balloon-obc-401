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

#include <functional>
#include <span>

#include <units/time.h>

#include "scheduling/task.hpp"

namespace obc::monitoring {
class WatchdogTask;

/**
 * @brief A task that is monitored by a watchdog.
 *
 * During the normal operation of a `WatchedTask`, a watchdog should be kicked
 * at a specific period. Failure to kick the watchdog for an extended period
 * indicates that a task has hung, whereas kicking it too frequently suggests
 * that the task may be stuck in a loop, repeatedly performing the same
 * incorrect operation. Tasks may also manually declare to the watchdog that
 * they are faulty.
 *
 * If a fault is detected, various interventions can be executed (configurable):
 * - Call a reset method on the task.
 * - Forcibly restart the task.
 * - Terminate the task.
 * - Launch a fallback version of the task.
 * - Reboot the system.
 *
 * TODO: Implement the interventions.
 *
 * Most tasks should be monitored by a watchdog.
 *
 * @warning Detection of a fault by the watchdog should never occur in regular
 * program flow. Internal fault detection and recovery is strongly preferred
 * wherever possible.
 * @warning This mechanism serves as an additional line of defense against
 * faults but should not be relied upon to detect all possible errors.
 */
class WatchedTask : public virtual scheduling::Task {
    friend WatchdogTask;

  protected:
    /**
     * @brief Kicks the watchdog and optionally flags the task as faulty.
     *
     * @param fault Indicates whether a fault has occurred.
     */
    inline void Kick(bool fault = false) {
        m_fault |= fault;
        if (m_fault) return;

        if (m_timer < MinKickPeriod() || m_timer > MaxKickPeriod()) {
            m_fault = true;
            return;
        }

        m_timer = units::microseconds<float>(0);
    }

    /**
     * @brief Specifies the minimum time allowed between watchdog kicks.
     *
     * Kicking the watchdog more frequently than this period is considered a
     * fault.
     */
    constexpr virtual units::microseconds<float> MinKickPeriod() const {
        return units::microseconds<float>(0);
    }

    /**
     * @brief Specifies the maximum time allowed between watchdog kicks.
     *
     * Failing to kick the watchdog within this period is considered a fault.
     */
    constexpr virtual units::microseconds<float> MaxKickPeriod() const {
        return units::microseconds<float>(0);
    }

  private:
    /**
     * @brief Advances the internal timer, indicating the elapsed time since the
     * last watchdog kick.
     *
     * TODO: Automate the advancement of the timer.
     *
     * @return Indicates whether the task is currently in a fault state.
     */
    inline bool Advance(units::microseconds<float> dt) {
        m_timer += dt;
        return m_fault;
    }

    units::microseconds<float> m_timer {0};
    bool                       m_fault {false};
};

constexpr std::uint32_t WatchdogStackSize = 512;

/**
 * @brief A task that monitors other tasks for faults and executes interventions
 * when needed.
 *
 * This task checks for faults in watched tasks and performs specified
 * interventions upon detection.
 */
class WatchdogTask : protected scheduling::StackTask<WatchdogStackSize>, public scheduling::Task {
  public:
    using Watchlist = std::span<std::reference_wrapper<WatchedTask>>;

    /**
     * @brief Creates a new watchdog task to monitor a set of tasks.
     *
     * @param watchlist The list of tasks to be monitored.
     */
    WatchdogTask(Watchlist watchlist) : scheduling::Task(m_task_stack), m_watchlist {watchlist} {}

  protected:
    /**
     * @brief Iterates through all watched tasks and performs interventions if
     * necessary.
     */
    inline void Run() override {
        for (auto& task_wrapper : m_watchlist) {
            auto& task = task_wrapper.get();
            // TODO: Properly calculate the time delta (dt)
            if (task.Advance(units::microseconds<float>(1000))) {
                // TODO: Implement task restart
                // TODO: Interface with the system watchdog upon failure of
                // critical tasks
            }
        }
    }

    constexpr const char* Name() const override { return "Watchdog"; }

    constexpr osPriority Priority() const override { return osPriorityHigh7; }

    /**
     * @warning The period should be set short enough to ensure interventions
     * occur promptly.
     */
    constexpr units::microseconds<float> NominalPeriod() const override {
        return units::microseconds<float>(1000);
    }

  private:
    Watchlist m_watchlist;
};
}  // namespace obc::monitoring
