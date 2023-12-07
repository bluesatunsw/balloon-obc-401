// Interface with I2S peripheral
// Decorator class to add I2S functionality peripheral

#ifndef I2S_HPP_
#define I2S_HPP_

#include "stm32f4xx_hal.h"

class I2C
{
public:
    I2C(I2C_HandleTypeDef* hi2c);
    ~I2C();

    // TODO: add read/write functions
    void write(uint16_t address, uint8_t* data, uint16_t size);
    void read(uint16_t address, uint8_t* data, uint16_t size);

private:
    I2C_HandleTypeDef* m_hi2c;
};