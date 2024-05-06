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

#pragma once

#include <deque>
#include <iterator>

namespace obc::port::mock {
template<typename T>
class MockInputPort {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    const T* operator->() const { return &m_data.front(); }

    const T& operator*() const { return m_data.front(); }

    MockInputPort& operator++() {
        m_data.pop_front();
        return *this;
    }

    void operator++(int) { operator++(); }

    void Push(T&& data) { m_data.push_back(data); }

    template<std::ranges::input_range<T> U>
    void Push(U&& data) {
        for (auto&& d : data) Push(d);
    }

    size_t RemainingData() { return m_data.size(); }

  private:
    std::deque<T> m_data;
};

template<typename T>
    requires std::is_default_constructible_v<T>
class MockOutputPort {
  public:
    using iterator_category = std::output_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    T* operator->() { return &m_data.front(); }

    T& operator*() const { return m_data.front(); }

    MockInputPort& operator++() {
        m_data.push_front(T());
        return *this;
    }

    void operator++(int) { operator++(); }

    T Pull() { m_data.pop_back(data); }

    template<std::ranges::output_range<T> U>
        requires std::ranges::bidirectional_range<T>
    void PullTo(U&& dest) {
        for (auto& d : std::ranges::reverse(dest)) d = Pull();
    }

    size_t RemainingData() { return m_data.size(); }

  private:
    std::deque<T> m_data;
};
}  // namespace obc::port::mock
