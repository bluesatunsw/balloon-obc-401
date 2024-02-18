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

#include <array>
#include <optional>
#include <span>

#include <units/time.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"

#include "scheduling/delay.hpp"

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
    /**
     * @brief Creates and starts a new task.
     */
    inline Task()
        : m_handle {xTaskCreateStatic(
              &RTOSTask, Name(), Stack().size(), this, Priority(),
              Stack().data(), &m_task_data
          )} {}

    Task(const Task& other) = delete;
    Task(Task&& other)      = delete;

    Task& operator=(const Task& other) = delete;
    Task& operator=(Task&& other)      = delete;

    /**
     * @brief Stops the underlying task immediately.
     */
    inline ~Task() { vTaskDelete(NULL); }

  protected:
    /**
     * @brief The function to be called periodically to execute the task.
     */
    virtual void Run() = 0;

    /**
     * @brief Gets the name of the task.
     *
     * Primarily used for logging and debugging purposes.
     */
    constexpr virtual const char* Name() const { return "Unnamed Task"; }

    /**
     * @brief Gets the memory to be used as the stack for this task.
     *
     * @warning If the stack is undersized, undefined behavior can occur.
     */
    virtual std::span<StackType_t> Stack() {
    	// Needed to prevent a linker error.
    	return {};
    };

    constexpr virtual osPriority Priority() const { return osPriorityNormal; }

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
    constexpr virtual units::microseconds<float> NominalPeriod() const {
        return units::microseconds<float>(0);
    }

  private:
    /**
     * @brief C-style wrapper function which can be invoked by FreeRTOS.
     *
     * Repeatedly invokes the periodic run function each NominalPeriod.
     */
    inline static void RTOSTask(void* instance) {
        // TODO: Eliminate extra layer of indirection
        auto task {static_cast<Task*>(instance)};
        while (true) {
        	scheduling::Timeout::Guard timeout {task->NominalPeriod()};
            task->Run();
        }

        vTaskDelete(NULL);
    }

    TaskHandle_t m_handle;
    StaticTask_t m_task_data;
};

constexpr std::uint32_t DefaultStackDepth = 4096;

/**
 * @brief Mixin class to statically create a fixed-size stack for a task.
 */
template<std::uint32_t StackDepth = DefaultStackDepth>
class StackTask : public virtual Task {
  protected:
    std::span<StackType_t> Stack() override final { return m_task_stack; }

  private:
    std::array<StackType_t, StackDepth> m_task_stack;
};
}  // namespace obc