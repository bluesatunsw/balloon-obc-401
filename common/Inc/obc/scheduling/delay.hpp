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

#include <concepts>
#include <optional>
#include <variant>

#include <units/time.h>

#include "FreeRTOS.h"
#include "task.h"
#include "utils/meta.hpp"

namespace obc::scheduling {
/**
 * @brief A concept defining an object that can be repeatedly called with no
 * arguments, yielding a specific type.
 *
 * @tparam R The desired return type.
 */
template<typename T, typename R>
concept TypedPollable = requires(T t) {
    { t() } -> std::convertible_to<R>;
};

/**
 * @brief A concept defining an object that can be repeatedly called with no
 * arguments, yielding an option type.
 */
template<typename T>
concept Pollable = requires(T t) {
    { t() } -> obc::utils::OptionLike;
};

/**
 * @brief A wrapper for FreeRTOS timeouts, providing high-level functions.
 *
 * Typically used for synchronously waiting for an event, such as a response
 * from a sensor on a bus. To prevent tasks from locking up, raw retry loops
 * should be refactored to utilize this class instead.
 *
 * TODO: Add the ability to yield after each failed attempt.
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

    class Guard;

    /**
     * @brief Runs a callable repeatedly until it yields a result or the timeout
     * period expires.
     *
     * If the callable returns a std::nullopt, this indicates that it should be
     * retried if time remains in the timeout. Any other return value is
     * treated as a success.
     *
     * @param f The callable object to invoke repeatedly.
     *
     * @return The result of the callable or std::nullopt if the timeout
     * expired.
     */
    template<Pollable F>
    auto Poll(F& f) -> std::optional<std::remove_reference_t<decltype(*f())>> {
        while (!(*this))
            if (auto x = f()) return *x;
        return std::nullopt;
    }

    /**
     * @brief Runs a callable repeatedly until it succeeds or the timeout period
     * expires.
     *
     * The return value of the callable indicates if it succeeded. False
     * indicates failure and that it should be retried if time remains in the
     * timeout.
     *
     * @param f The callable object to invoke repeatedly.
     *
     * @return True if the callable succeeded before the timeout.
     */
    template<TypedPollable<bool> F>
    bool Poll(F& f) {
        return Poll<std::monostate>([&] {
            if (f()) return std::monostate {};
            return std::nullopt;
        });
    }

  private:
    TimeOut_t  m_timeout;
    TickType_t m_period;
};

/**
 * @brief An RAII wrapper around a delay.
 *
 * Used to ensure a block of code takes a specific amount of time to execute,
 * useful for basic sequencing. It is more accurate than inserting a raw delay
 * since it accounts for the execution time of operations within the block.
 *
 * @code
 * {
 *     obc::Timeout::Guard block_delay{10_ms};
 *     SendDataToMotors();
 *     obc::Timeout::Guard post_send_delay{5_ms};
 * }
 * @endcode
 * In the above code, the block will always take at least 10ms to run,
 * regardless of how long `SendDataToMotors` takes. There will also be a delay
 * of at least 5ms after the call to `SendDataToMotors`.
 *
 * The delay occurs when the object goes out of scope, allowing for a raw delay
 * to be implemented by creating a guard without storing it.
 *
 * TODO: Log when a timeout "overruns" (the period has already elapsed before
 * the end of the block).
 */
class Timeout::Guard {
  public:
    /**
     * @brief Converts a timeout into a guard.
     *
     * This can be used to consume remaining time leftover from a polling
     * operation.
     *
     * @param timeout The timeout to convert.
     */
    explicit Guard(Timeout timeout);

    /**
     * @brief Creates a delay guard to wait for a specified period.
     *
     * @param period The duration of the delay.
     */
    explicit Guard(units::microseconds<float> period);

    /**
     * @brief Blocks for the remaining duration of the timeout.
     */
    ~Guard();

  private:
    Timeout m_timeout;
};
}  // namespace obc::scheduling
