#include <concepts>
#include <expected>

#include "utils/meta.hpp"

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

#define UNWRAP_ASSIGN_IMPL(lhs, rexpr, temp)                                 \
    auto temp = rexpr;                                                  \
    static_assert(obc::utils::ExpectedReturn<std::remove_cvref<temp>>); \
    if constexpr (!std::same_as<                                        \
                      std::remove_cvref<temp>::error_type,              \
                      obc::utils::Never>) {                             \
        if (!temp) return std::unexpected(std::move(temp).error());     \
    }                                                                   \
    lhs = std::move(temp.value());

#define UNWRAP_ASSIGN(lhs, rexpr) UNWRAP_ASSIGN_IMPL(lhs, rexpr, TEMP_NAME)
}  // namespace obc::utils
