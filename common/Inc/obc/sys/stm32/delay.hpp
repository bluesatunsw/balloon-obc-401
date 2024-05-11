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

#include <FreeRTOS.h>
#include <units/time.h>

namespace obc::scheduling::detail {
/**
 * @brief A wrapper for FreeRTOS timeouts.
 */
class Timeout {
  public:
    /**
     * @brief Creates a new timeout with a specified timeout period.
     *
     * Timeouts start immediately upon creation.
     *
     * @param period The duration of the timeout.
     */
    explicit Timeout(units::microseconds<float> period);

    /**
     * @brief Checks if the timeout period has elapsed.
     *
     * @return True if the timeout has elapsed.
     */
    operator bool();

    /**
     * @brief Waits for the remaining duration of the timeout.
     *
     * @warning Currently, this is implemented as a pure busy loop.
     *
     * TODO: Switch to a hybrid approach: busy loop for short periods and yield
     * to the system scheduler for longer periods.
     */
    void Block();

  protected:
    /**
     * @brief Hint to the scheduler that it should probably context switch to
     * another task.
     *
     * This is allowed to be implemented as a no-op.
     *
     * @warning A best effort is made to return control before the timeout has
     * elapsed. However, when polling for a condition a final check should be
     * performed after the final yield.
     */
    void Yield();

  private:
    TimeOut_t  m_timeout;
    TickType_t m_period;
};
}  // namespace obc::scheduling::detail
