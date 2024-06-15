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

#include <concepts>
#include <expected>

#include "obc/utils/meta.hpp"

namespace obc::utils {
/**
 * @brief A class which implements the error interface or
 * is the special Never type (ie. not an error).
 *
 * @todo Implment the error interface, currently only `Never`
 * is valid.
 *
 * @warning MaybeError is not a valid return type for a function
 * which may fail or return nothing. You probably want to use
 * `std::optional<Error>` instead.
 */
template<typename T>
concept MaybeError = std::same_as<T, Never>;

template<typename T, typename R>
concept ExpectedReturn =
    Specializes<T, std::expected> && MaybeError<typename T::error_type> &&
    std::convertible_to<T, std::expected<R, typename T::error_type>>;

/*This functionality is impossible is impossible to implement without
 * macros since unwrapping is an operation which can return from the
 * enclosing context.
 */
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNWRAP(expr)                                                          \
    ({                                                                        \
        auto res {expr};                                                      \
        static_assert(obc::utils::Specializes<decltype(res), std::expected>); \
        if constexpr (!std::same_as<                                          \
                          typename std::remove_cvref<decltype(res             \
                          )>::error_type,                                     \
                          obc::utils::Never>) {                               \
            if (!res) return std::unexpected(std::move(res.error()));         \
        }                                                                     \
        res.value();                                                          \
    })
// NOLINTEND(cppcoreguidelines-macro-usage)
}  // namespace obc::utils
