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
#include "delay.hpp"
#include "task.h"

namespace obc {
class Task {
  public:
    inline Task()
        : m_handle {xTaskCreateStatic(
              &RTOSTask, Name(), Stack().size(), this, Priority(),
              Stack().data(), &m_task_data
          )} {}

    Task(const Task& other) = delete;
    Task(Task&& other)      = delete;

    Task& operator=(const Task& other) = delete;
    Task& operator=(Task&& other)      = delete;

    ~Task() = default;

  protected:
    virtual void Run() = 0;

    constexpr virtual const char* Name() const { return "Unnamed Task"; }

    inline virtual std::span<StackType_t> Stack() { return {}; }

    constexpr virtual osPriority Priority() const { return osPriorityNormal; }

    constexpr virtual units::microseconds<float> NominalPeriod() const {
        return units::microseconds<float>(0);
    }

  private:
    inline static void RTOSTask(void* instance) {
        while (true) {
            // TODO: Eliminate extra layer of indirection
            auto           task = static_cast<Task*>(instance);
            Timeout::Guard timeout {task->NominalPeriod()};
            task->Run();
        }

        vTaskDelete(NULL);
    }

    TaskHandle_t m_handle;
    StaticTask_t m_task_data;
};

constexpr std::uint32_t DefaultStackDepth = 4096;

template<std::uint32_t StackDepth = DefaultStackDepth>
class StackTask : public virtual Task {
  protected:
    std::span<StackType_t> Stack() override final { return m_task_stack; }

  private:
    std::array<StackType_t, StackDepth> m_task_stack;
};
}  // namespace obc
