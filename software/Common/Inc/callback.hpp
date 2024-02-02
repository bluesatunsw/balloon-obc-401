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
    using MethodWrapperPtr = R (*)(void*, void*, As...);
    template<typename T>
    using MethodPtr = R (T::*)(As...);
    template<typename D, typename T>
        requires(sizeof(D) == sizeof(void*))
    using MethodPtrData = R(T::*)(D, As...);

    R operator()(As... args) { return m_method(m_callee, m_data, args...); }

    template<typename T, MethodPtr<T> M>
    Callback(T& instance)
        : m_callee(&instance), m_method(&MethodWrapper<T, M>) {}

    template<typename D, typename T, MethodPtr<T> M>
        requires(sizeof(D) == sizeof(void*))
    Callback(T& instance, D data)
        : m_callee(&instance), m_method(&MethodWrapperData<D, T, M>),
          m_data(reinterpret_cast<void*>(data)) {}

  private:
    template<typename T, MethodPtr<T> M>
    static void MethodWrapper(void* callee, void* data, As... args) {
        (static_cast<T*>(callee)->*M)(args...);
    }

    template<typename D, typename T, MethodPtrData<D, T> M>
    static void MethodWrapperData(void* callee, void* data, As... args) {
        (static_cast<T*>(callee)->*M)(reinterpret_cast<D>(data), args...);
    }

    void*            m_callee {nullptr};
    MethodWrapperPtr m_method {nullptr};
    void*            m_data {nullptr};
};
}  // namespace obc
