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

#include <stm32h7xx_hal_def.h>

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
 * `std::expected<std::monostate, Error>` instead.
 */
template<typename T>
concept MaybeError = std::same_as<T, Never>;

template<typename T, typename R>
concept ExpectedReturn =
    Specializes<T, std::expected> && MaybeError<typename T::error_type> &&
    std::convertible_to<T, std::expected<R, typename T::error_type>>;

template<typename T, typename U>
concept PredicateFunction = requires(T& f, const U& x) {
    { f(x) } -> std::convertible_to<bool>;
};

/*
 * This functionality is impossible is impossible to implement without
 * macros since unwrapping is an operation which can return from the
 * enclosing context.
 */
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNWRAP(expr)                                                          \
    ({                                                                        \
        auto res {expr};                                                      \
        static_assert(obc::utils::Specializes<decltype(res), std::expected>); \
        if constexpr (!std::same_as<                                          \
                          typename std::remove_cvref_t<decltype(res           \
                          )>::error_type,                                     \
                          obc::utils::Never>) {                               \
            if (!res) return std::unexpected {std::move(res.error())};        \
        }                                                                     \
        res.value();                                                          \
    })

#define UNWRAP_TAGGED(expr, tag)                                                                                                       \
    ({                                                                                                                                 \
        auto res {expr};                                                                                                               \
        static_assert(obc::utils::Specializes<decltype(res), std::expected> || obc::utils::Specializes<decltype(res), std::optional>); \
        if constexpr (obc::utils::Specializes<decltype(res), std::optional>) {                                                         \
            if (!res) return std::unexpected {tag};                                                                                    \
        } else if constexpr (!std::same_as<                                                                                            \
                                 typename std::remove_cvref_t<decltype(res                                                             \
                                 )>::error_type,                                                                                       \
                                 obc::utils::Never>) {                                                                                 \
            if (!res)                                                                                                                  \
                return std::unexpected {                                                                                               \
                    {tag, std::move(res.error())}                                                                                      \
                };                                                                                                                     \
        }                                                                                                                              \
        res.value();                                                                                                                   \
    })

#define UNWRAP_OPT(expr)                                                      \
    ({                                                                        \
        auto res {expr};                                                      \
        static_assert(obc::utils::Specializes<decltype(res), std::optional>); \
        if (!res) return std::nullopt;                                        \
        res.value();                                                          \
    })

// NOLINTEND(cppcoreguidelines-macro-usage)

template<OptionLikeAny T>
inline auto UnwrapOrPanic(T x) -> std::remove_reference_t<decltype(*x)> {
    if (static_cast<bool>(x)) return *x;
    Panic();
}

template<typename T, PredicateFunction<T> C>
inline auto CheckOrPanic(T x, C& check) -> T {
    if (check(x)) return x;
    Panic();
}

inline auto [[noreturn]] Panic() -> void {
    // TODO(evan): make this do something
}

inline auto IsHalOk(const HAL_StatusTypeDef status) -> bool {
    return status == HAL_OK;
}
}  // namespace obc::utils
