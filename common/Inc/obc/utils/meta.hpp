#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace obc::utils {
template<typename T>
concept TupleLike = requires {
    std::tuple_size_v<T>;
    typename std::tuple_element_t<0, T>;
};

template<typename T>
concept OptionLike = requires(T t) {
    { static_cast<bool>(t) };
    { *t };
};

template<std::size_t I, template<typename...> typename F, TupleLike... Ts>
struct MultiwayConjunction : std::conjunction<
                                 F<std::tuple_element_t<I - 1, Ts>...>,
                                 MultiwayConjunction<I - 1, F, Ts...>> {};

template<template<typename...> typename F, TupleLike... Ts>
struct MultiwayConjunction<0, F, Ts...> : std::true_type {};

template<typename F, typename T>
concept AllPairsConvertable =
    TupleLike<F> && TupleLike<T> &&
    (std::tuple_size_v<F> == std::tuple_size_v<T>)&&MultiwayConjunction<
        std::tuple_size_v<F>, std::is_convertible, F, T>::value;

template<typename... Ts>
class TypeAmalgam : public Ts... {};
}  // namespace obc::utils
