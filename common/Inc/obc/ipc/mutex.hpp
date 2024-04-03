/* USER CODE BEGIN Header */
/*
 * 401 Ballon OBC
 * Copyright (C) 2023 Bluesat and contributors
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

#include "FreeRTOS.h"
#include "semphr.h"

namespace obc::ipc {
/**
 * @brief C++ wrapper around FreeRTOS mutex to make it a Lockable.
 *
 * If there are no other special requirements, this should be the default
 * mutex used.
 */
class Mutex {
  public:
    Mutex();

    /**
     * @brief Locks the mutex, blocking if necessary.
     */
    void lock();

    /**
     * @brief Unlocks the mutex.
     */
    void unlock();

    /**
     * @brief Attempts to lock the mutex without blocking.
     *
     * @return True if the lock was acquired successfully, false otherwise.
     */
    bool try_lock();

  private:
    SemaphoreHandle_t m_handle;
    StaticSemaphore_t m_data;
};

/**
 * @brief RAII wrapper for marking a block of code as a critical section.
 *
 * In a critical section, all interrupts are disabled to prevent concurrent
 * access to shared resources from interrupt service routines. This ensures
 * that the wrapped code executes atomically.
 *
 * @warning This class disables scheduler preemption, which can cause the system
 * to lock up if used improperly.
 */
class CriticalGuard {
  public:
    /**
     * @brief Enters the critical section by disabling interrupts.
     */
    CriticalGuard();

    /**
     * @brief Leaves the critical section by re-enabling interrupts.
     */
    ~CriticalGuard();
};

/**
 * @brief Basic spinlock implementation compatible with the Lockable concept.
 *
 * Designed for low contention scenarios involving very short operations, where
 * the overhead of acquiring a conventional mutex would be disproportionate.
 * Guarded code sections effectively become critical sections to prevent
 * context switching while a spinlock is held. This avoids prolonged spinning
 * by other tasks attempting to acquire the lock.
 *
 * @warning Does not yield to the system scheduler while the lock is or
 * attempting to be acquired..
 */
class SpinLock {
  public:
    /**
     * @brief Locks the spinlock, actively waiting if necessary.
     */
    void lock();

    /**
     * @brief Unlocks the spinlock.
     */
    void unlock();

    /**
     * @brief Attempts to lock the spinlock without blocking.
     *
     * @return True if the lock was acquired successfully, false otherwise.
     */
    bool try_lock();

  private:
    std::atomic_flag m_lock {false};
};
}  // namespace obc::ipc
