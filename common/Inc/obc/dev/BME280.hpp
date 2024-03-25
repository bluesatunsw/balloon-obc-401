#pragma once

#include <units/time.h>

#include "bus.hpp"
#include "scheduling/task.hpp"
#include "weather.hpp"

using units::literals;

namespace obc::sensors {
template<typename T>
    requires obc::bus::RequestBus<T>
class BME280 {
  public:
    BME280(T& bus, std::uint8_t address) : m_bus {bus}, m_address {address} {
        // Read calibration data
        AsyncValue<BasicMessage> async_result;
        m_bus.Request(
            {address : m_address, data : {0x88}}, async_result,
            bus::StructAsBuffer(m_calibration_lo)
        );
        if (!Await(async_result, 10_ms)) {
            // TODO: Handle error
        }

        m_bus.Request(
            {address : m_address, data : {0xA1}}, async_result,
            bus::StructAsBuffer(dig_H1)
        );
        if (!Await(async_result, 10_ms)) {
            // TODO: Handle error
        }

        m_bus.Request(
            {address : m_address, data : {0xE1}}, async_result,
            bus::StructAsBuffer(m_calibration_hi)
        );
        if (!Await(async_result, 10_ms)) {
            // TODO: Handle error
        }

        opc::ipc::AsyncValue<BasicMessage> async_result;
        m_bus.Request(
            {address : m_address, data : {0xF7}},
            OBC_CALLBACK_METHOD(*this, HandleData),
            bus::StructAsBuffer(m_raw_data)
        );
    }

    units::celsius<float> GetTemperature() const {
        return m_temperature.load();
    }

    units::pascals<float> GetPressure() const { return m_pressure.load(); }

    float GetHumidity() const { return m_humidity.load(); }

  private:
    T&           m_bus;
    std::uint8_t m_address;

    constexpr units::time::milliseconds kRepeatPeriod = 10_ms;

    // Compensation Parameters
    struct __attribute__((packed)) {
        std::uint16_t dig_T1;
        std::int16_t  dig_T2;
        std::int16_t  dig_T3;
        std::uint16_t dig_P1;
        std::int16_t  dig_P2;
        std::int16_t  dig_P3;
        std::int16_t  dig_P4;
        std::int16_t  dig_P5;
        std::int16_t  dig_P6;
        std::int16_t  dig_P7;
        std::int16_t  dig_P8;
        std::int16_t  dig_P9;
    } m_calibration_lo;

    std::uint8_t dig_H1;

    struct __attribute__((packed)) {
        std::int16_t dig_H2;
        std::uint8_t dig_H3;
        std::int16_t dig_H4;
        std::int16_t dig_H5;
        std::int8_t  dig_H6;
    } m_calibration_hi;

    struct {
        unsigned int adc_P : 20;
        unsigned int pad1  : 4;
        unsigned int adc_T : 20;
        unsigned int pad2  : 4;
        unsigned int adc_H : 16;
    } m_raw_data;

    /// Used in compensation calculations for humidity and pressure
    std::int32_t m_temperature_fine;

    std::atomic<units::celsius<float>> m_temperature;
    std::atomic<units::pascals<float>> m_pressure;
    std::atomic<float>                 m_humidity;

    void HandleData(const BasicMessage& message) {
        m_temperature.store(BME280_compensate_T_int32(m_raw_data.adc_T));
        m_pressure.store(BME280_compensate_P_int64(m_raw_data.adc_P));
        m_humidity.store(BME280_compensate_H_int32(m_raw_data.adc_H));

        AsyncValue<BasicMessage> async_result;
        m_bus.Request(
            {address : m_address, data : {0xF7}},
            OBC_CALLBACK_METHOD(*this, HandleData),
            bus::StructAsBuffer(m_raw_data)
        );

        scheduling::Timeout(kRepeatPeriod).Block();
    }

    /// Terrifying functions come from
    /// https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
    units::celsius<float> BME280_compensate_T_int32(std::int32_t adc_T) {
        std::int32_t var1, var2, T;
        var1 = (((adc_T >> 3) -
                 (static_cast<std::int32_t>(m_raw_data.dig_T1) << 1)) *
                static_cast<std::int32_t>(m_raw_data.dig_T2)) >>
               11;
        var2 =
            (((((adc_T >> 4) - static_cast<std::int32_t>(m_raw_data.dig_T1)) *
               ((adc_T >> 4) - static_cast<std::int32_t>(m_raw_data.dig_T1))) >>
              12) *
             static_cast<std::int32_t>(m_raw_data.dig_T3)) >>
            14;

        m_temperature_fine = var1 + var2;
        T                  = (m_temperature_fine * 5 + 128) >> 8;
        return units::celsius<float>(static_cast<float>(T) / 100);
    }

    units::pascals<float> BME280_compensate_P_int64(std::int32_t adc_P) {
        std::int64_t var1, var2, p;
        var1 = static_cast<std::int64_t>(m_temperature_fine) - 128000;
        var2 = var1 * var1 * static_cast<std::int64_t>(m_raw_data.dig_P6);
        var2 = var2 +
               ((var1 * static_cast<std::int64_t>(m_raw_data.dig_P5)) << 17);
        var2 = var2 + ((static_cast<std::int64_t>(m_raw_data.dig_P4)) << 35);
        var1 = ((var1 * var1 * static_cast<std::int64_t>(m_raw_data.dig_P3)) >>
                8) +
               ((var1 * static_cast<std::int64_t>(m_raw_data.dig_P2)) << 12);
        var1 = ((1L << 47) + var1) *
                   static_cast<std::int64_t>(m_raw_data.dig_P1) >>
               33;
        if (var1 == 0) return 0;  // avoid exception caused by division by zero
        p    = 1048576 - adc_P;
        p    = ((p << 31) - var2) * 3125 / var1;
        var1 = (static_cast<std::int64_t>(m_raw_data.dig_P9) * (p >> 13) *
                (p >> 13)) >>
               25;
        var2 = (static_cast<std::int64_t>(m_raw_data.dig_P8) * p) >> 19;
        p    = (p + var1 + var2) >>
            8 + (static_cast<std::int64_t>(m_raw_data.dig_P7) << 4);
        return units::pascals<float>(static_cast<float>(p) / 256);
    }

    float BME280_compensate_H_int32(std::int32_t adc_H) {
        std::int32_t v_x1_u32r;
        v_x1_u32r = m_temperature_fine - static_cast<std::int32_t>(76800);
        v_x1_u32r =
            ((((adc_H << 14) -
               (static_cast<std::int32_t>(m_raw_data.dig_H4) << 20) -
               (static_cast<std::int32_t>(m_raw_data.dig_H5) * v_x1_u32r)) +
              16384) >>
             15) *
            (((((((v_x1_u32r * static_cast<std::int32_t>(m_raw_data.dig_H6)) >>
                  10) *
                 (((v_x1_u32r * static_cast<std::int32_t>(m_raw_data.dig_H3)) >>
                   11) +
                  32768)) >>
                10) +
               2097152) *
                  static_cast<std::int32_t>(m_raw_data.dig_H2) +
              8192) >>
             14);
        v_x1_u32r =
            v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                          static_cast<std::int32_t>(m_raw_data.dig_H1)) >>
                         4);
        v_x1_u32r = v_x1_u32r < 0 ? 0 : v_x1_u32r;
        v_x1_u32r = v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r;
        return static_cast<float>(v_x1_u32r) / 1024.0;
    }
};
}  // namespace obc::sensors

// static_assert(obc::sensors::TemperatureSensor<
//               obc::sensors::BME280<obc::bus::I2C>>);
// static_assert(obc::sensors::HumiditySensor<
//               obc::sensors::BME280<obc::bus::I2C>>);
// static_assert(obc::sensors::PressureSensor<
//               obc::sensors::BME280<obc::bus::I2C>>);
