/*
 * LIS3.c
 *
 *  Created on: May 5, 2024
 *      Author: Nate Hunter
 */
#include "LIS3.h"
static SPI_HandleTypeDef *spi;
static GPIO_TypeDef *_NSS_Port;
static uint16_t _NSS_Pin;
static uint8_t txbuf[1];
static uint8_t _mode = LIS_STBY;
static float _scale;
uint8_t LIS3_Init(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin){
	spi = spiHandle;
	_NSS_Port = NSS_Port;
	_NSS_Pin = NSS_Pin;
	txbuf[0] = LIS_READ_BIT | LIS_WHO_AM_I;
	_NSS_Port->ODR &= ~_NSS_Pin;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	HAL_SPI_Receive(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	if(*txbuf != 0x3D) return 0;
	return 1;
}

void LIS3_Config(uint8_t reg, uint8_t cfg){
	/*
	 * CTRL1: |TEMP_EN|OM1,OM0|DO2,DO1,DO0|FAST_ODR|ST|
	 * CTRL2: |0|FS1,FS0|00|REBOOT|SOFT_RST|00|
	 * CTRL3: |00|LP|00|SIM|MD1,MD0|
	 */
	if(reg == LIS_CTRL3) _mode = cfg & 0b11;

	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = LIS_CTRL_REG1 + reg;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	txbuf[0] = cfg;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;

	if(reg == LIS_CTRL1) { //copy OM to Z-axis
		_NSS_Port->ODR &= ~_NSS_Pin;
		txbuf[0] = LIS_CTRL_REG1 + 3;
		HAL_SPI_Transmit(spi, txbuf, 1, 1000);
		txbuf[0] = (cfg >> 3) & (0b11 << 2);
		HAL_SPI_Transmit(spi, txbuf, 1, 1000);
		_NSS_Port->ODR |= _NSS_Pin;
	}

	if(reg == LIS_CTRL2) {		//@formatter:off
		switch(cfg&LIS_SCALE_16){
			case LIS_SCALE_4: _scale = 6842.0 * 10e-3; break;
			case LIS_SCALE_8: _scale = 3421.0 * 10e-3; break;
			case LIS_SCALE_12: _scale = 2281.0 * 10e-3; break;
			case LIS_SCALE_16: _scale = 1711.0 * 10e-3; break;
			default: break;
		}								//@formatter:on
	}
}

void LIS3_Read(float *mag){
	txbuf[0] = LIS_READ_BIT | LIS_STATUS;
	uint8_t status = 0;
	int16_t x, y, z;
	if(_mode == LIS_SINGLE) LIS3_Config(LIS_CTRL3, LIS_SINGLE);
	while(!(status & (1 << 3))) {
		_NSS_Port->ODR &= ~_NSS_Pin;
		HAL_SPI_Transmit(spi, txbuf, 1, 1000);
		HAL_SPI_Receive(spi, &status, 1, 1000);
		_NSS_Port->ODR |= _NSS_Pin;
	}
	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = LIS_READ_BIT | LIS_MULTY_BIT | LIS_OUT_X_L;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	HAL_SPI_Receive(spi, (uint8_t*) &x, 2, 1000);
	HAL_SPI_Receive(spi, (uint8_t*) &y, 2, 1000);
	HAL_SPI_Receive(spi, (uint8_t*) &z, 2, 1000);
	mag[0] = x / _scale;
	mag[1] = y / _scale;
	mag[2] = z / _scale;
	_NSS_Port->ODR |= _NSS_Pin;
}
