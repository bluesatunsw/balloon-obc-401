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

#include "delay.hpp"

namespace obc {
Timeout::Timeout(units::quantised::Microseconds period)
    : m_period(units::quantised::ToTicks(period)) {
    vTaskSetTimeOutState(&m_timeout);
}

bool Timeout::operator bool() {
    return xTaskCheckForTimeOut(&time_out, &ticks_to_wait) == pdTRUE;
}

void Timeout::Block() {
    while (!(*this)) {}
}

Timeout::Guard::Guard(Timeout timeout) : m_timeout(timeout) {}

Timeout::Guard::Guard(units::quantised::Microseconds period)
    : m_timeout({period}) {}

Timeout::Guard::~Guard() { m_timeout.Block(); }
}  // namespace obc
