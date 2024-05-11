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

#include <iterator>

/**
 * @brief Defines a common interface for IO ports.
 *
 * A port is an abstract source or sink of discrete units of data. For example
 * a digital pin would be a port for booleans and a serial port would be a port
 * for chars.
 *
 * Ports are similar to input/output iterators with the type primarily being
 * used to signal intent, however there may be additional constraints placed on
 * them in future.
 *
 * @warning Data is only pulled/pushed to the port upon calling the increment
 * operator, this operation may block for an unspecified duration.
 *
 * @warning Polling of a port must be externally rate limited.
 *
 * @warning Given the nature of the advance operation, it is generally unsafe
 * for a port to be directly shared.
 */
namespace obc::port {
/**
 * @brief A port which acts as a source of data.
 *
 * @tparam D Type of data which is pulled from this port.
 */
template<typename T, typename D>
concept InputPort =
    std::semiregular<D> && std::input_iterator<T> &&
    std::convertible_to<typename std::iterator_traits<T>::value_type, D>;

/**
 * @brief A port which acts as a sink of data.
 *
 * @tparam D Type of data which is pushed into this port.
 */
template<typename T, typename D>
concept OutputPort = std::semiregular<D> && std::output_iterator<T, D>;
}  // namespace obc::port
