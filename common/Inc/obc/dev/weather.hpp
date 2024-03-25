#include <concepts>

#include <units/pressure.h>
#include <units/temperature.h>

template<typename T>
concept TemperatureSensor = requires(const T& sensor) {
    { sensor.GetTemperature() } -> std::convertible_to<units::celsius<float>>;
};

template<typename T>
concept HumiditySensor = requires(const T& sensor) {
    { sensor.GetHumidity() } -> std::convertible_to<float>;
};

template<typename T>
concept PressureSensor = requires(const T& sensor) {
    { sensor.GetPressure() } -> std::convertible_to<units::pascals<float>>;
};
