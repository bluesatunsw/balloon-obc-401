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

#include <atomic>
#include <mutex>

#include "FreeRTOS.h"
#include "semphr.h"

namespace obc {
class Mutex {
  public:
    Mutex();

    void lock();
    void unlock();
    bool try_lock();

  private:
    SemaphoreHandle_t m_handle;
    StaticSemaphore_t m_data;
};

class CriticalGuard {
  public:
    CriticalGuard();
    ~CriticalGuard();
};

class SpinLock {
  public:
    void lock();
    void unlock();
    bool try_lock();

  private:
    std::atomic_flag m_lock {false};
};
}  // namespace obc
