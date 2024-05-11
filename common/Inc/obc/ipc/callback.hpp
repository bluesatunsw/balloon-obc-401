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

#include <atomic>
#include <concepts>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <variant>

#include <units/time.h>

#include "obc/ipc/mutex.hpp"
#include "obc/scheduling/delay.hpp"

namespace obc::ipc {
/**
 * @brief Wrapper type for data which can be asynchronously set.
 *
 * Polling operation can be performed to check/wait for the value.
 *
 * @tparam T Type of wrapped value.
 * @tparam L Lock to use for thread safety.
 */
template<typename T, typename L = SpinLock>
class AsyncValue {
  public:
    AsyncValue() = default;

    /**
     * @brief Assign a result.
     *
     * @param data Value to set.
     *
     * @warning It is undefined behaviour to assign to an async value twice
     */
    void Set(const T& data) {
        std::scoped_lock lock(m_lock);
        if (!m_data) m_data = data;
    }

    /**
     * @brief Get the underlying value if it exists.
     *
     * @return std::nullopt if the value has not yet be set, otherwise a
     * reference to the value.
     */
    std::optional<std::reference_wrapper<T>> operator()() {
        // Returning a reference to the underlying option would be unsafe
        std::scoped_lock lock(m_lock);
        return m_data ? std::optional<std::reference_wrapper<T>>(m_data.value())
                      : std::optional<std::reference_wrapper<T>>(std::nullopt);
    }

  private:
    std::optional<T> m_data {};
    L                m_lock {};
};

/**
 * @brief A type that may be used as the bound argument to a callback.
 *
 * To ensure that callbacks can be copied and moved without any knowledge of
 * the type of data bound, only data which is trivially copyable and of the
 * same size as a void* is permitted.
 *
 * void* was chosen as it ensures that, on most platforms, references can be
 * stored and, for other kinds of data, it typically corresponds to a size
 * that the system can quickly manipulate.
 */
template<typename T>
concept CallbackData = requires {
    sizeof(T) == sizeof(void*);
    std::is_trivially_copyable_v<T>;
};

namespace internal {
/**
 * @brief Flag type to indicate a callback with no bound data.
 */
struct NoDataFlag {
    void* pad {nullptr};
};

/**
 * @brief Helper container which can eventually be cast into a callback.
 *
 * @tparam Mt Type of callback method.
 * @tparam M Callback method.
 * @tparam T Class to which the method belongs.
 * @tparam D Type of the additional argument.
 */
template<typename Mt, Mt M, typename T, CallbackData D = NoDataFlag>
struct CallbackProvider {
    CallbackProvider(T& inst, D data) : i_(inst), d_(data) {};
    explicit CallbackProvider(T& inst) : i_(inst), d_() {};

    Mt M_ {M};
    T& i_;
    D  d_;
};
}  // namespace internal

// TODO: Determine if this is possible with some other method such as template
// deduction guides instead.

/**
 * @brief Creates a temporary object which can be converted to a callback when
 * required.
 *
 * @param inst Instance object which this callback is on.
 * @param fn Member function of callback.
 */
#define OBC_CALLBACK_METHOD(inst, fn)                       \
    obc::ipc::internal::CallbackProvider<                   \
        decltype(&std::remove_cvref_t<decltype(inst)>::fn), \
        &std::remove_cvref_t<decltype(inst)>::fn,           \
        std::remove_cvref_t<decltype(inst)>>(inst)

/**
 * @see OBC_CALLBACK_METHOD
 *
 * Also binds a curries the method with a piece of data passed to the function
 * as the first argument.
 *
 * @param data_type Type of the data to be curried.
 * @param data Value of the curried data.
 */
#define OBC_CALLBACK_CURRIED_METHOD(inst, fn, data_type, data) \
    obc::ipc::internal::CallbackProvider<                      \
        decltype(&std::remove_cvref_t<decltype(inst)>::fn),    \
        &std::remove_cvref_t<decltype(inst)>::fn,              \
        std::remove_cvref_t<decltype(inst)>, data_type>(inst, data)

/**
 * @brief A member function pointer bound to an object.
 *
 * Behaves as a standard Callable object.
 *
 * Unlike std::bind, this does not perform any dynamic allocation because it
 * can only be bound with a pointer to an object and optionally a single
 * argument.
 *
 * @tparam R Return type of the callback.
 * @tparam As Arguments passed to the callback (excluding the optional bound
 * one).
 */
template<typename R, typename... As>
class Callback {
  public:
    /**
     * @brief Invokes the callback function.
     *
     * @param args List of arguments to be passed to the callback.
     *
     * @return Result of invoking the callback.
     */
    R operator()(As... args) { return m_method(m_callee, m_data, args...); }

    /**
     * @brief Convert a callback provider wrapper into an actual callback type.
     *
     * @param prov Provider to translate to callback
     *
     * @tparam Mt Type of the method pointe
     * @tparam M Method pointer
     * @tparam T Instance type
     * @tparam D Bound data type
     *
     * @warning Should not be directly called.
     */
    template<typename Mt, Mt M, typename T, typename D>
    explicit(false) Callback(internal::CallbackProvider<Mt, M, T, D> prov)
        requires(std::same_as<D, internal::NoDataFlag>)
        : m_method(&MethodWrapper<Mt, M, T>), m_callee(&prov.i_) {}

    /**
     * @see Callback::Callback
     */
    template<typename Mt, Mt M, typename T, typename D>
    explicit(false) Callback(internal::CallbackProvider<Mt, M, T, D> prov)
        requires(!std::same_as<D, internal::NoDataFlag>)
        : m_method(MethodWrapper<Mt, M, T, D>), m_callee(&prov.i_),
          m_data(reinterpret_cast<void*>(prov.d_)) {}

    /**
     * @brief Convert an async value to a callback which sets it.
     *
     * @param async Async wrapper the result of the callback will be written to.
     *
     * @tparam T Underlying type of async value.
     * @tparam L Lock used by async wrapper.
     */
    template<typename T, typename L = SpinLock>
    explicit(false) Callback(AsyncValue<T, L>& async)
        : m_method(&MethodWrapper<
                   decltype(&AsyncValue<T, L>::Set), &AsyncValue<T, L>::Set,
                   AsyncValue<T, L>>),
          m_callee(&async) {}

    /**
     * @brief Dummy class which contain a function convertible to this kind
     * of callback.
     *
     * This is useful for defining constraints for function which take callbacks
     * so as to allow the "native" callback type to have differing argument and
     * return types to the one implied by the concept. Such a construction is
     * required as, unlike `std::function`, it is impossible to cast between
     * callbacks even if their constituent types are implicitly convertible
     * between each other, despite creating a callback to a method with
     * convertible types being a permitted operation.
     *
     * Impossible to instantiate directly or indirectly.
     */
    class DummySource {
      public:
        /**
         * @brief Function which can be converted.
         */
        virtual R Func(As... args) final = 0;
    };

    /**
     * @brief Callback provider associated with this type's dummy source.
     */
    using DummyProvider = internal::CallbackProvider<
        decltype(&DummySource::Func), &DummySource::Func, DummySource>;

  private:
    /*
     * Internally, C-style function pointers are used to store the function to
     * be called; as a result, they must be plain C-like functions (i.e., not a
     * member). Member functions are thus called indirectly via a wrapper
     * function which has C++ type information but a basic signature.
     *
     * The wrapper function casts the generic pointer to a pointer to the
     * instance class so that the class method can be invoked. If data is
     * bound to this callback, it is cast back to the correct type and passed
     * as the first argument to the function. The remaining arguments are left
     * unchanged.
     */

    template<typename Mt, Mt M, typename T>
    static R MethodWrapper(void* callee, void* data, As... args) {
        return (static_cast<T*>(callee)->*M)(args...);
    }

    template<typename Mt, Mt M, typename T, CallbackData D>
    static R MethodWrapper(void* callee, void* data, As... args) {
        return (static_cast<T*>(callee)->*M)(
            reinterpret_cast<D>(data), args...
        );
    }

    using MethodWrapperPtr = R (*)(void*, void*, As...);
    /// C-style function pointer to the wrapper function.
    MethodWrapperPtr m_method {nullptr};
    /// Generic pointer to the object to which this method is bound.
    void*            m_callee {nullptr};
    /// Generic storage for the additional bound argument.
    void*            m_data {nullptr};
};

template<typename T, typename R, typename... As>
concept CallbackSource = std::convertible_to<T, Callback<R, As...>>;

template<typename T, typename F, typename... As>
std::optional<T> Await(F f, units::milliseconds<float> timeout, As... args) {
    AsyncValue<T> res {};
    f(res, args...);
    return obc::scheduling::Timeout(timeout).Poll(res);
}
}  // namespace obc::ipc
