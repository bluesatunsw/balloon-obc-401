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

#include <array>
#include <concepts>
#include <deque>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "obc/port.hpp"

/**
 * @brief Mocks for IO ports.
 *
 * The lack of GMock mock methods is an intentional design choice. In the spirit
 * of black box testing, only side effects which are observable in a real system
 * should be tested.
 */
namespace obc::port::mock {
/**
 * @brief Mock of \ref InputPort.
 *
 * Test data can be pushed into this object which is then pulled by the classes
 * under test. Data is pushed and pulled as a FIFO.
 *
 * @warning Should not be used to test classes where the rate at which data is
 * pulled is an implementation detail. See \ref MockRepeatingInputPort instead.
 *
 * @tparam D Type of data which is pulled from this port.
 */
template<std::semiregular T>
class MockInputPort {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    /**
     * @brief Get the current value of the port.
     *
     * @throws std::out_of_range if there is no more data to be read.
     *
     * @return Pointer to the current value.
     */
    const T* operator->() const {
        if (!RemainingData()) throw std::out_of_range("Port has no more data.");

        return &m_data.front();
    }

    /**
     * @brief Get the current value of the port.
     *
     * @throws std::out_of_range if there is no more data to be read.
     *
     * @return Reference to the current value.
     */
    const T& operator*() const {
        if (!RemainingData()) throw std::out_of_range("Port has no more data.");

        return m_data.front();
    }

    /**
     * @brief Pull the next unit of data from the port.
     *
     * @throws std::out_of_range upon attempting to advance past the empty
     * state.
     *
     * @return Reference to this port.
     */
    MockInputPort& operator++() {
        if (!RemainingData())
            throw std::out_of_range("Advancing port beyond empty state.");

        m_data.pop_front();
        return *this;
    }

    /**
     * @brief Pull the next unit of data from the port.
     *
     * @throws std::out_of_range upon attempting to advance past the empty
     * state.
     */
    void operator++(int) { operator++(); }

    /**
     * @brief Push mock data to the port.
     *
     * @param data Unit of data to be pushed.
     */
    void Push(T&& data) { m_data.push_back(data); }

    /**
     * @brief Push a range of mock data to the port.
     *
     * @param data Range to draw data from.
     *
     * @tparam U Type of the input range.
     */
    template<std::ranges::input_range<T> U>
    void Push(U&& data) {
        for (auto&& d : data) Push(d);
    }

    /**
     * @brief Gets the amount of data that has been pushed, but not pulled.
     *
     * @return Number of units of data.
     */
    size_t RemainingData() { return m_data.size(); }

  private:
    std::deque<T> m_data {};
};

/**
 * @brief Mock of \ref InputPort.
 *
 * A constant value or provider function is used to determine the value of data
 * when the class under test pulls data. This is useful for testing objects
 * which having polling like behaviour, ie. the rate at which they pull data is
 * an implementation detail.
 *
 * @tparam D Type of data which is pulled from this port.
 */
template<std::semiregular T>
class MockRepeatingInputPort {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    MockRepeatingInputPort(std::function<T()>&& fn)
        : m_provider(fn), m_cache(m_provider()) {}

    MockRepeatingInputPort(T&& val)
        : MockRepeatingInputPort([=]() { return val; }) {}

    /**
     * @brief Get the current value of the port.
     *
     * @return Pointer to the current value.
     */
    const T* operator->() const { return &m_cache; }

    /**
     * @brief Get the current value of the port.
     *
     * @return Reference to the current value.
     */
    const T& operator*() const { return m_cache; }

    /**
     * @brief Pull the next unit of data from the port.
     *
     * @return Reference to this port.
     */
    MockRepeatingInputPort& operator++() { m_cache = m_provider(); }

    /**
     * @brief Pull the next unit of data from the port.
     */
    void operator++(int) { operator++(); }

  private:
    std::function<T()> m_provider;
    T                  m_cache;
};

/**
 * @brief Mock of \ref OutputPort.
 *
 * The data pushed into this port by objects under test can be pulled
 * out and verified. Data is pushed and pulled as a FIFO.
 *
 * @tparam D Type of data which is pushed into this port.
 */
template<std::semiregular T>
class MockOutputPort {
  public:
    using iterator_category = std::output_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    /**
     * @brief Get the current value of the port.
     *
     * @return Pointer to the current value.
     */
    T* operator->() { return &m_data.front(); }

    /**
     * @brief Get the current value of the port.
     *
     * @return Pointer to the current value.
     */
    T& operator*() const { return m_data.front(); }

    /**
     * @brief Push the current unit of data into the port.
     *
     * @return Reference to this port.
     */
    MockInputPort& operator++() {
        m_data.push_front(T());
        return *this;
    }

    /**
     * @brief Push the current unit of data into the port.
     */
    void operator++(int) { operator++(); }

    /**
     * @brief Pull (get and remove) a single unit of data from the port.
     *
     * @throws std::out_of_range if there is no more data to be read.
     *
     * @return Oldest data from the port.
     */
    T Pull() {
        if (!RemainingData()) throw std::out_of_range("Port has no more data.");

        auto res = m_data.back();
        m_data.pop_back(data);
        return res;
    }

    /**
     * @brief Pull (get and remove) a multiple units of data from the port.
     *
     * @throws std::out_of_range if more data was requested than exists.
     *
     * @return Array of the oldest data from the port.
     *
     * @tparam N Amount of data to pull.
     */
    template<size_t N>
    std::array<T, N> PullN() {
        std::array<T, N> res;
        for (size_t i = 0; i < N; i++) res[i] m_data.pop_back(data);
        return res;
    }

    /**
     * @brief Pull (get and remove) a multiple units of data from the port into
     * a range.
     *
     * Data is pulled until the end of the range is reach or the data from the
     * port is exhausted. This is done in reverse order so that the order which
     * the data was pushed matches with the order of the data in the output.
     *
     * @param dest Range to write data to.
     *
     * @return Amount of data pulled.
     *
     * @tparam U Type of output range.
     */
    template<std::ranges::output_range<T> U>
        requires std::ranges::bidirectional_range<T>
    size_t PullTo(U&& dest) {
        for (auto& d : std::ranges::reverse(dest)) {
            if (!RemainingData()) break;

            d = Pull();
        }
    }

    /**
     * @brief Get and clear all data which has been pushed to this port.
     *
     * @return All outstanding data which has been pushed to the port.
     */
    std::vector<T> Drain() {
        std::vector<T> res;
        res.reserve(RemainingData());
        res.append_range(std::move(m_data));

        m_data = {T()};
        return res;
    }

    /**
     * @brief Gets the amount of data that has been pushed, but not pulled.
     *
     * @return Number of units of data.
     */
    size_t RemainingData() { return m_data.size(); }

  private:
    std::deque<T> m_data {T()};
};
}  // namespace obc::port::mock
