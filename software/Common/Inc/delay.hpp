#pragma once

#include <concepts>
#include <optional>
#include <variant>

#include "FreeRTOS.h"

#include "units.hpp"

namespace obc {
template<typename T, typename R>
concept Pollable = requires(T t) {
    { t() } -> std::convertible_to<R>;
};

class Timeout {
  public:
    explicit Timeout(units::quantised::Microseconds period);

    bool operator bool();
    void Block();

    class Guard {
      public:
        explicit Guard(Timeout timeout);
        explicit Guard(units::quantised::Microseconds period);
        ~Guard();

      private:
        Timeout m_timeout;
    };

    template<typename R, Pollable<std::optional<R>> F>
    static std::optional<R> Poll(F f, units::quantised::Microseconds timeout) {
        Timeout timer {timeout};
        while (!timer)
            if (auto x = f()) return *x;
        return std::nullopt;
    }

    template<Pollable<bool> F>
    static bool Poll(F f, units::quantised::Microseconds timeout) {
        return Poll<std::monostate>(
            [&] {
                if (f()) return std::monostate {};
                return std::nullopt;
            },
            timeout
        );
    }

  private:
    TimeOut_t  m_timeout;
    TickType_t m_period;
};
}  // namespace obc
