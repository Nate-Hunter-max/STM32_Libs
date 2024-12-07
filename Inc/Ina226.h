/*
 * Ina226.h
 *
 *  Created on: Dec 03, 2024
 *      Author: Nate Hunter
 */
#pragma once
#include "main.h"

#define INA226_DEFAULT_ADDRESS 0x40

#define INA226_SHUNT_RESISTOR 0.01f
#define INA226_MAX_CURRENT 1.0f


typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    float shunt_resistor;
    uint16_t calibration;
} INA226_HandleTypeDef;

HAL_StatusTypeDef INA226_Init(INA226_HandleTypeDef *ina);
HAL_StatusTypeDef INA226_ReadBusVoltage(INA226_HandleTypeDef *ina, float *voltage);
HAL_StatusTypeDef INA226_ReadCurrent(INA226_HandleTypeDef *ina, float *current);
