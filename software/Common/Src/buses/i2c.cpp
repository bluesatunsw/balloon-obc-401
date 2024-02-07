/* USER CODE BEGIN Header */
/*
 * 401 Balloon OBC
 * Copyright (C) 2024 BLUEsat and contributors
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

#include "buses/i2c.hpp"
#include "../../Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_i2c.h"

#define MAX_WAIT 1000

namespace obc::bus {

void I2CBus::Send(BasicMessage msg, int busNumber) {
	//decide which bus, based on address
	if (busNumber <= 2 && busNumber >= 1) {
		I2C_HandleTypeDef *i2CHandle = NULL;
		if (busNumber == 1) {
			i2CHandle = &hi2c1;
		} else {
			i2CHandle = &hi2c2;
		}
	} else {
		// TODO throw error
	}

	HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(i2CHandle, msg.Address, msg.data, 1, MAX_WAIT); //Sending in Blocking mode
	if (ret != HAL_OK) {
	      // TODO throw error
	}
	HAL_Delay(100);
}


} // namespace obc::bus
