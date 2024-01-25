#include "delay.hpp"

#include <units/frequency.h>

namespace obc {
Timeout::Timeout(units::microseconds<float> period)
    : m_period(static_cast<TickType_t>((period / units::hertz<float>(configTICK_RATE_HZ)).value())) {
    vTaskSetTimeOutState(&m_timeout);
}

Timeout::operator bool() {
    return xTaskCheckForTimeOut(&m_timeout, &m_period) == pdTRUE;
}

void Timeout::Block() {
    while (!(*this)) {}
}

Timeout::Guard::Guard(Timeout timeout) : m_timeout(timeout) {}

Timeout::Guard::Guard(units::microseconds<float> period)
    : m_timeout(period) {}

Timeout::Guard::~Guard() { m_timeout.Block(); }
}  // namespace obc
