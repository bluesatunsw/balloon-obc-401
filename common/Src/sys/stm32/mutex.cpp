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

#include "obc/ipc/mutex.hpp"

namespace obc::ipc {
Mutex::Mutex() : m_handle {xSemaphoreCreateMutexStatic(&m_data)} {}

auto Mutex::lock() -> void { xSemaphoreTake(m_handle, portMAX_DELAY); }

auto Mutex::unlock() -> void { xSemaphoreGive(m_handle); }

auto Mutex::try_lock() -> bool {
    return xSemaphoreTake(m_handle, static_cast<TickType_t>(0)) == pdTRUE;
}

CriticalGuard::CriticalGuard() { taskENTER_CRITICAL(); }

CriticalGuard::~CriticalGuard() { taskEXIT_CRITICAL(); }

auto SpinLock::lock() -> void {
    taskENTER_CRITICAL();
    while (m_lock.test_and_set(std::memory_order_acquire)) {
        while (m_lock.test(std::memory_order_relaxed)) {}
    }
}

auto SpinLock::unlock() -> void {
    m_lock.clear(std::memory_order_release);
    taskEXIT_CRITICAL();
}

auto SpinLock::try_lock() -> bool {
    taskENTER_CRITICAL();
    if (m_lock.test_and_set(std::memory_order_acquire)) {
        // Failed to acquire
        taskEXIT_CRITICAL();
        return false;
    }
    return true;
}
}  // namespace obc::ipc
