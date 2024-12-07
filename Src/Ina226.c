#include "Ina226.h"
#define CONFIG_REGISTER 0x00
#define CALIBRATION_REGISTER 0x05
#define SHUNT_VOLTAGE_REGISTER 0x01
#define BUS_VOLTAGE_REGISTER 0x02
#define CURRENT_REGISTER 0x04

#define DEFAULT_CONFIG 0x4127

#define BUS_VOLTAGE_LSB 0.00125f
#define CURRENT_LSB(calibration, shunt) (0.00512f / ((float)(calibration) * (shunt)))

uint16_t txTemp;

HAL_StatusTypeDef INA226_Init(INA226_HandleTypeDef *ina) {
    uint16_t config = DEFAULT_CONFIG;
    ina->calibration = (uint16_t)(0.00512f / ((INA226_MAX_CURRENT / 32768.0f)*ina->shunt_resistor));

    txTemp = SWAP_UINT16(config);
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(ina->hi2c, ina->address << 1, CONFIG_REGISTER, 1, (uint8_t *)&txTemp, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    txTemp = SWAP_UINT16(ina->calibration);
    status = HAL_I2C_Mem_Write(ina->hi2c, ina->address << 1, CALIBRATION_REGISTER, 1, (uint8_t *)&txTemp, 2, HAL_MAX_DELAY);
    return status;
}

HAL_StatusTypeDef INA226_ReadBusVoltage(INA226_HandleTypeDef *ina, float *voltage) {
    uint16_t raw_data = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(ina->hi2c, ina->address << 1, BUS_VOLTAGE_REGISTER, 1, (uint8_t *)&raw_data, 2, HAL_MAX_DELAY);
    if (status == HAL_OK) {
    	raw_data = SWAP_UINT16(raw_data);
        *voltage = raw_data * BUS_VOLTAGE_LSB;
    }
    return status;
}

HAL_StatusTypeDef INA226_ReadCurrent(INA226_HandleTypeDef *ina, float *current) {
    uint16_t raw_data = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(ina->hi2c, ina->address << 1, CURRENT_REGISTER, 1, (uint8_t *)&raw_data, 2, HAL_MAX_DELAY);
    if (status == HAL_OK) {
    	raw_data = SWAP_UINT16(raw_data);
        *current = raw_data * CURRENT_LSB(ina->calibration, ina->shunt_resistor);
    }
    return status;
}

