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

#include <array>
#include <optional>
#include <span>

#include <units/time.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "obc/scheduling/delay.hpp"
#include "task.h"

namespace obc::scheduling {
/**
 * @brief Wrapper around FreeRTOS tasks to adhere to object-oriented
 * conventions.
 *
 * A task object must override certain properties required by FreeRTOS. For
 * simplicity, these properties are immutable and known at compile time.
 */
class Task {
  public:
    // This is interfacing with C-Style FreeRTOS code which uses out
    // parameters to initialise values
    // NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    /**
     * @brief Creates and starts a new task.
     */
    inline Task(std::span<StackType_t> stack)
        : m_handle {xTaskCreateStatic(
              &RTOSTask, Name(), stack.size(), this, Priority(), stack.data(),
              &m_task_data
          )} {}

    // NOLINTEND(cppcoreguidelines-pro-type-member-init,hicpp-member-init)

    Task(const Task& other) = delete;
    Task(Task&& other)      = delete;

    auto operator=(const Task& other) -> Task& = delete;
    auto operator=(Task&& other) -> Task&      = delete;

    /**
     * @brief Stops the underlying task immediately.
     */
    virtual inline ~Task() { vTaskDelete(NULL); }

  protected:
    /**
     * @brief The function to be called periodically to execute the task.
     */
    virtual auto Run() -> void = 0;

    /**
     * @brief Gets the name of the task.
     *
     * Primarily used for logging and debugging purposes.
     */
    [[nodiscard]] constexpr virtual auto Name() const -> const char* {
        return "Unnamed Task";
    }

    [[nodiscard]] constexpr virtual auto Priority() const -> osPriority {
        return osPriorityNormal;
    }

    /**
     * @brief Gets the length of the period of time between re-executions of the
     * `Run` function.
     *
     * @warning This time period is actually the time before the task is allowed
     * to be rerun, so the true period between executions will be slightly
     * longer.
     *
     * TODO: Add compensation so that the average period will tend towards the
     * `NominalPeriod`.
     */
    [[nodiscard]] constexpr virtual auto NominalPeriod() const
        -> units::microseconds<float> {
        return units::microseconds<float>(1000);
    }

  private:
    /**
     * @brief C-style wrapper function which can be invoked by FreeRTOS.
     *
     * Repeatedly invokes the periodic run function each NominalPeriod.
     */
    inline static auto RTOSTask(void* instance) -> void {
        // TODO(evan): Eliminate extra layer of indirection
        auto* task {static_cast<Task*>(instance)};
        while (true) {
            scheduling::Timeout::Guard timeout {task->NominalPeriod()};
            task->Run();
        }

        vTaskDelete(NULL);
    }

    TaskHandle_t m_handle;
    StaticTask_t m_task_data;
};

constexpr std::uint32_t kDefaultStackDepth = 4096;

/**
 * @brief Mixin class to statically create a fixed-size stack for a task.
 */
template<std::uint32_t StackDepth = kDefaultStackDepth>
class StackTask {
  protected:
    std::array<StackType_t, StackDepth> m_task_stack {};
};
}  // namespace obc::scheduling
