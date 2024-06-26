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
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace obc::utils {
/**
 * @brief A type which has indexible elements whose type can be obtained at
 * compile time with `std::tuple_element_t`.
 */
template<typename T>
concept TupleLike = requires {
    { std::tuple_size_v<T> } -> std::convertible_to<size_t>;
    typename std::tuple_element_t<0, T>;
};

template<typename T>
concept NonVoid = !std::same_as<T, void>;

/**
 * @brief A type which has a truthiness associated with an instance being
 * convertable to a particular type with the deref operator.
 *
 * `std::optional<U>`, `std::expected<U, _>` and `U*` statisfy this.
 *
 * @tparam U Type to which this is convertable to.
 */
template<typename T, typename U>
concept OptionLike = requires(T t) {
    { static_cast<bool>(t) };
    { *t } -> std::convertible_to<U>;
};

/**
 * @brief A type which has a truthiness associated with an instance being
 * convertable to some value with the deref operator.
 *
 * `std::optional<T>`, `std::expected<T, _>` and `T*` statisfy this.
 */
template<typename T>
concept OptionLikeAny = requires(T t) {
    { static_cast<bool>(t) };
    { *t } -> NonVoid;
};

/**
 * @brief An appoximation of a 🥺 type.
 *
 * 🥺 types are used to represent values which cannot exist in contexts
 * where a type must be specified. In other words, a type for which there
 * are no valid values.
 *
 * Unlike a true 🥺 type (like those in Rust), this cannot be cast into
 * any other type. This was an intentional design choice to prevent the
 * accidental creation and
 *
 * This type can be privately subclassed by other marker types to prevent
 * construction and use.
 *
 * It is possible to move and destruct a never type purely to allow templates
 * to keep working.
 */
struct Never {
    Never()                                = delete;
    Never(const Never&)                    = delete;
    auto operator=(const Never&) -> Never& = delete;
    Never(Never&&)                         = default;
    auto operator=(Never&&) -> Never&      = default;
    ~Never()                               = default;
};

/**
 * @brief Check if a predicate is true for all elements of the zip of the
 * element types of tuples.
 *
 * @tparam I Current index of the element type in each tuple to check.
 * @tparam F Predicate function.
 * @tparam Ts Pack of all the tuples to zip.
 */
template<std::size_t I, template<typename...> typename F, TupleLike... Ts>
struct MultiwayConjunction : std::conjunction<
                                 F<std::tuple_element_t<I - 1, Ts>...>,
                                 MultiwayConjunction<I - 1, F, Ts...>> {};

template<template<typename...> typename F, TupleLike... Ts>
struct MultiwayConjunction<0, F, Ts...> : std::true_type {};

/**
 * @brief Helper variable tempalate for \ref MultiwayConjunction.
 */
template<template<typename...> typename F, TupleLike T, TupleLike... Ts>
constexpr bool MultiwayConjunctionV =
    ((std::tuple_size_v<T> == std::tuple_size_v<Ts>)&&...) &&
    MultiwayConjunction<std::tuple_size_v<T>, F, Ts...>::value;

/**
 * @brief Check if all element types in a tuple can be converted to their
 * corresponding element types in another.
 *
 * @tparam F Tuple of types to convert from.
 * @tparam T Tuple of types to convert from.
 */
template<typename F, typename T>
concept AllPairsConvertable = MultiwayConjunctionV<std::is_convertible, F, T>;

/**
 * @brief Create a type which is derived from a pack of other types.
 */
template<typename... Ts>
class TypeAmalgam : public Ts... {};

template<typename T, template<typename...> class Z>
struct IsSpecializationOf : std::false_type {};

template<typename... Args, template<typename...> class Z>
struct IsSpecializationOf<Z<Args...>, Z> : std::true_type {};

template<typename T, template<typename...> class Z>
inline constexpr bool IsSpecializationOfV = IsSpecializationOf<T, Z>::value;

template<typename T, template<typename...> class Z>
concept Specializes = IsSpecializationOfV<T, Z>;
}  // namespace obc::utils
