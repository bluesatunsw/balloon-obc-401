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

namespace obc {
template<typename T, typename R>
concept Pollable = requires(T t) {
    { t() } -> std::convertible_to<R>;
};

class Timeout {
  public:
    explicit Timeout(units::microseconds<float> period);

    operator bool();
    void Block();

    class Guard;

    template<typename R, Pollable<std::optional<R>> F>
    static std::optional<R> Poll(F f, units::microseconds<float> timeout) {
        Timeout timer {timeout};
        while (!timer)
            if (auto x = f()) return *x;
        return std::nullopt;
    }

    template<Pollable<bool> F>
    static bool Poll(F f, units::microseconds<float> timeout) {
        return Poll<std::monostate>(
            [&] {
                if (f()) return std::monostate {};
                return std::nullopt;
            },
            timeout
        );
    }

  private:
    TimeOut_t  m_timeout;
    TickType_t m_period;
};

class Timeout::Guard {
  public:
    explicit Guard(Timeout timeout);
    explicit Guard(units::microseconds<float> period);
    ~Guard();

  private:
    Timeout m_timeout;
};
}  // namespace obc
