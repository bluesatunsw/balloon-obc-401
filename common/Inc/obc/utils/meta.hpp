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

template<template<typename...> typename F, typename... Ts>
constexpr bool MultiwayConjunctionV = false;

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
}  // namespace obc::utils
