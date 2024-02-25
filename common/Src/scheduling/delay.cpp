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

#include "scheduling/delay.hpp"

#include <units/frequency.h>

namespace obc::scheduling {
Timeout::Timeout(units::microseconds<float> period)
    : m_period(static_cast<TickType_t>(
          (period / units::hertz<float>(configTICK_RATE_HZ)).value()
      )) {
    vTaskSetTimeOutState(&m_timeout);
}

Timeout::operator bool() {
    return xTaskCheckForTimeOut(&m_timeout, &m_period) == pdTRUE;
}

void Timeout::Block() {
    while (!(*this)) {}
}

Timeout::Guard::Guard(Timeout timeout) : m_timeout(timeout) {}

Timeout::Guard::Guard(units::microseconds<float> period) : m_timeout(period) {}

Timeout::Guard::~Guard() { m_timeout.Block(); }
}  // namespace obc::scheduling
