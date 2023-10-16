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

#include "../Inc/mutex.hpp"

namespace obc {
Mutex::Mutex() : m_handle {xSemaphoreCreateMutexStatic(&m_data)} {}

void Mutex::lock() { xSemaphoreTake(m_handle, portMAX_DELAY); }

void Mutex::unlock() { xSemaphoreGive(m_handle); }

bool Mutex::try_lock() {
    return xSemaphoreTake(m_handle, static_cast<TickType_t>(0)) == pdTRUE;
}
}  // namespace obc
