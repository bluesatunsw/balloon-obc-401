#include "delay.hpp"

namespace obc {
Timeout::Timeout(units::quantised::Microseconds period)
    : m_period(units::quantised::ToTicks(period)) {
    vTaskSetTimeOutState(&m_timeout);
}

bool Timeout::operator bool() {
    return xTaskCheckForTimeOut(&time_out, &ticks_to_wait) == pdTRUE;
}

void Timeout::Block() {
    while (!(*this)) {}
}

Timeout::Guard::Guard(Timeout timeout) : m_timeout(timeout) {}

Timeout::Guard::Guard(units::quantised::Microseconds period)
    : m_timeout({period}) {}

Timeout::Guard::~Guard() { m_timeout.Block(); }
}  // namespace obc
