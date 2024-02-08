/* USER CODE BEGIN Header */
/*
 * 401 Ballon OBC
 * Copyright (C) 2023 BLUEsat and contributors
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

#include <cstdint>
#include <type_traits>

namespace obc::ipc {
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
	std::is_trivially_copyable<T>::value;
};

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
     * @brief Pointer to a member function of the correct argument type and
     * return type.
     *
     * @tparam T Class to which the method belongs.
     *
     * @warning The types must match exactly; implicit casting is not performed.
     */
    template<typename T>
    using MethodPtr = R (T::*)(As...);

    /**
     * @brief Pointer to a member function of the correct argument type and
     * return type, with an additional argument that will be bound and
     * precedes the unbound argument.
     *
     * @tparam D Type of the additional argument.
     *
     * @tparam T Class to which the method belongs.
     *
     * @warning The types must match exactly; implicit casting is not performed.
     */
    template<CallbackData D, typename T>
    using MethodPtrData = R (T::*)(D, As...);

    /**
     * @brief Invokes the callback function.
     *
     * @param args List of arguments to be passed to the callback.
     *
     * @return Result of invoking the callback.
     */
    R operator()(As... args) { return m_method(m_callee, m_data, args...); }

    /**
     * @brief Creates a new callback bound to a specific class instance.
     *
     * @tparam T Class to which the method belongs.
     * @tparam M Callback method.
     *
     * @param instance Object to which this callback is bound.
     */
    template<typename T, MethodPtr<T> M>
    explicit(false) Callback(T& instance)
        : m_callee(&instance), m_method(&MethodWrapper<T, M>) {}

    /**
     * @brief Creates a new callback bound to a specific class instance and
     * piece of data (as an additional argument).
     *
     * @tparam D Type of the additional argument.
     * @tparam T Class to which the method belongs.
     * @tparam M Callback method.
     *
     * @param instance Object to which this callback is bound.
     * @param data Additional argument (precedes all others).
     */
    template<CallbackData D, typename T, MethodPtr<T> M>
    Callback(T& instance, D data)
        : m_callee(&instance), m_method(&MethodWrapperData<D, T, M>),
          m_data(reinterpret_cast<void*>(data)) {}

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

    template<typename T, MethodPtr<T> M>
    static void MethodWrapper(void* callee, void* data, As... args) {
        (static_cast<T*>(callee)->*M)(args...);
    }

    template<CallbackData D, typename T, MethodPtrData<D, T> M>
    static void MethodWrapperData(void* callee, void* data, As... args) {
        (static_cast<T*>(callee)->*M)(reinterpret_cast<D>(data), args...);
    }

    using MethodWrapperPtr = R (*)(void*, void*, As...);
    /// C-style function pointer to the wrapper function.
    MethodWrapperPtr m_method {nullptr};
    /// Generic pointer to the object to which this method is bound.
    void*            m_callee {nullptr};
    /// Generic storage for the additional bound argument.
    void*            m_data {nullptr};
};
}  // namespace obc::ipc
