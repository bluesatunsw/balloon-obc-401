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

#include <cstdint>

namespace obc {
template<typename R, typename... As>
class Callback {
  public:
    using MethodWrapperPtr = R (*)(void*, std::uint64_t, As...);
    template<typename T>
    using MethodPtr = R (T::*)(As...);
    template<typename T, typename... Us>
    using MethodPtrAlt = R (T::*)(Us...);

    R operator()(As... args) { return m_method(m_callee, m_data, args...); }

  protected:
    Callback(void* callee, MethodWrapperPtr method, std::uint64_t data)
        : m_callee {callee}, m_method {method}, m_data {data} {}

  private:
    void*            m_callee {nullptr};
    MethodWrapperPtr m_method {nullptr};
    std::uint64_t    m_data {};
};
}  // namespace obc
