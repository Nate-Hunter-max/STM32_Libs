/*
 * LSM6.h
 *
 *  Created on: May 6, 2024
 *      Author: Nate Hunter
 */

#pragma once
#include "main.h"
#define LSM6_READ_BIT (1<<7)
#define LSM6_WHO_AM_I 0x0F
#define LSM6_OUTX_L_G 0x22
#define LSM6_CTRL1_XL 0x10

#define LSM6_ACCEL_2G 0b00<<2
#define LSM6_ACCEL_4G 0b10<<2
#define LSM6_ACCEL_8G 0b11<<2
#define LSM6_ACCEL_16G 0b01<<2

#define LSM6_GYRO_245DPS 0b00<<2
#define LSM6_GYRO_500DPS 0b01<<2
#define LSM6_GYRO_1000DPS 0b10<<2
#define LSM6_GYRO_2000DPS 0b11<<2

#define LSM6_CFG_STBY (0b0000<<4)
#define LSM6_CFG_1_6_Hz (0b1011<<4)
#define LSM6_CFG_12_5_Hz (0b0001<<4)
#define LSM6_CFG_26_Hz (0b0010<<4)
#define LSM6_CFG_52_Hz (0b0011<<4)
#define LSM6_CFG_104_Hz (0b0100<<4)
#define LSM6_CFG_208_Hz (0b0101<<4)
#define LSM6_CFG_416_Hz (0b0110<<4)
#define LSM6_CFG_833_Hz (0b0111<<4)
#define LSM6_CFG_1_66_kHz (0b1000<<4)
#define LSM6_CFG_3_33_kHz (0b1001<<4)
#define LSM6_CFG_6_66_kHz (0b1010<<4)
static const float lsm6SensA[4] = { 0.061, 0.488, 0.122, 0.244 };
static const float lsm6SensG[4] = { 8.75, 17.5, 35.0, 70.0 };

uint8_t LSM6_Init(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin);
void LSM6_Read(float *accel, float *gyro);
void LSM6_ConfigAG(uint8_t A_CFG, uint8_t G_CFG);
