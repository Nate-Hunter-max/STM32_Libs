/*
 * LSM6.c
 *
 *  Created on: May 6, 2024
 *      Author: Nate Hunter
 */
#include "LSM6.h"

static SPI_HandleTypeDef *spi;
static GPIO_TypeDef *_NSS_Port;
static uint16_t _NSS_Pin;
static uint16_t _fsAccel;
static uint16_t _fsGyro;
static uint8_t txbuf[1];

uint8_t LSM6_Init(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin){
	spi = spiHandle;
	_NSS_Port = NSS_Port;
	_NSS_Pin = NSS_Pin;
	txbuf[0] = LSM6_READ_BIT | LSM6_WHO_AM_I;
	_NSS_Port->ODR &= ~_NSS_Pin;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	HAL_SPI_Receive(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	if(*txbuf != 0x6A) return 0;

	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = LSM6_CTRL1_XL + 2;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	txbuf[0] = (1 << 6) | 1; //enable Block Data Update in ctrl3;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	return 1;

}

void LSM6_ConfigAG(uint8_t A_CFG, uint8_t G_CFG){ //config: FS|ODR
	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = LSM6_CTRL1_XL;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	HAL_SPI_Transmit(spi, &A_CFG, 1, 1000);
	HAL_SPI_Transmit(spi, &G_CFG, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	_fsAccel = A_CFG >> 2 & 0b11;
	_fsGyro = G_CFG >> 2 & 0b11;
}

void LSM6_Read(float *accel, float *gyro){
	_NSS_Port->ODR &= ~_NSS_Pin;
	int16_t temp[3];
	txbuf[0] = LSM6_READ_BIT | LSM6_OUTX_L_G;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);

	for(*txbuf = 0; *txbuf < 3; (*txbuf)++) {
		HAL_SPI_Receive(spi, (uint8_t*) (temp + *txbuf), 2, 1000);
		gyro[*txbuf] = (float) temp[*txbuf] * lsm6SensG[_fsGyro];
	}

	for(*txbuf = 0; *txbuf < 3; (*txbuf)++) {
		HAL_SPI_Receive(spi, (uint8_t*) (temp + *txbuf), 2, 1000);
		accel[*txbuf] = (float) temp[*txbuf] * lsm6SensA[_fsAccel];
	}

	_NSS_Port->ODR |= _NSS_Pin;
}
