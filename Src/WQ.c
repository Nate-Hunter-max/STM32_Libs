/*
 * WQ.c
 *
 *  Created on: Jun 9, 2024
 *      Author: Nate Hunter
 */
#include "WQ.h"
static SPI_HandleTypeDef *spi;
static uint8_t txbuf[10];
static uint8_t rxbuf[10];
static GPIO_TypeDef *_NSS_Port;
static uint16_t _NSS_Pin;

uint8_t _wq_ready(uint32_t timeout);
uint8_t _wq_Write_Page(uint32_t adr, uint8_t *buf, uint32_t len);
uint8_t _wq_status();

uint8_t _wq_ready(uint32_t timeout){
	uint32_t begin = HAL_GetTick();
	uint32_t now = HAL_GetTick();
	while((now - begin <= timeout) && (_wq_status() && 0x01)) {
		now = HAL_GetTick();
	}
	if(now - begin >= timeout) return 0;
	return 1;
}

uint8_t _wq_status(){
	txbuf[0] = 0x05;
	_NSS_Port->ODR &= ~_NSS_Pin;
	HAL_SPI_TransmitReceive(spi, txbuf, rxbuf, 2, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	return rxbuf[1];
}

uint8_t _wq_Write_Page(uint32_t adr, uint8_t *buf, uint32_t len){
	if(!_wq_ready(1000)) return 0;

	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = WQ_CMD_WRITE_EN;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;

	txbuf[0] = WQ_CMD_PAGE_PROGRAM;
	txbuf[1] = (uint8_t) (adr >> 16);
	txbuf[2] = (uint8_t) (adr >> 8);
	txbuf[3] = (uint8_t) (adr);

	_NSS_Port->ODR &= ~_NSS_Pin;
	HAL_SPI_Transmit(spi, txbuf, 4, 1000);
	HAL_SPI_Transmit(spi, buf, len, 1000);
	_NSS_Port->ODR |= _NSS_Pin;

	return 1;
}

uint8_t WQ_Write(uint32_t adr, void *buf, uint32_t len){
	uint32_t offset = adr % WQ_PAGE_SIZE;
	uint32_t remaining = len;
	uint32_t to_write;

	while(remaining > 0) {
		to_write = WQ_PAGE_SIZE - offset;
		if(to_write > remaining) {
			to_write = remaining;
		}
		if(!_wq_Write_Page(adr, buf, to_write)) {
			return 0;
		}
		adr += to_write;
		buf += to_write;
		remaining -= to_write;
		offset = 0;
	}

	return 1;
}

uint8_t WQ_Read(uint32_t adr, uint8_t *buf, uint32_t len){
	if(!_wq_ready(1000)) return 0;
	txbuf[0] = WQ_CMD_READ;
	txbuf[1] = (uint8_t) (adr >> 16);
	txbuf[2] = (uint8_t) (adr >> 8);
	txbuf[3] = (uint8_t) (adr);
	_NSS_Port->ODR &= ~_NSS_Pin;
	HAL_SPI_Transmit(spi, txbuf, 4, 1000);
	HAL_SPI_Receive(spi, buf, len, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	return 1;
}

uint8_t WQ_Erace(uint32_t address, uint32_t len){

	uint32_t fSec = address / WQ_SECTOR_SIZE; //4KB sector
	uint32_t lSec = (address + len) / WQ_SECTOR_SIZE;

	for(uint32_t sector = fSec; sector <= lSec; ++sector) {
		if(!_wq_ready(1000)) return 0;

		_NSS_Port->ODR &= ~_NSS_Pin;
		txbuf[0] = WQ_CMD_WRITE_EN;
		HAL_SPI_Transmit(spi, txbuf, 1, 1000);
		_NSS_Port->ODR |= _NSS_Pin;

		uint32_t adr = sector * WQ_SECTOR_SIZE;
		txbuf[0] = WQ_CMD_SECTOR_ERASE;
		txbuf[1] = (uint8_t) (adr >> 16);
		txbuf[2] = (uint8_t) (adr >> 8);
		txbuf[3] = (uint8_t) (adr);

		_NSS_Port->ODR &= ~_NSS_Pin;
		HAL_SPI_Transmit(spi, txbuf, 4, 1000);
		_NSS_Port->ODR |= _NSS_Pin;
	}
	return 1;
}

uint8_t WQ_Chip_Erace(){
	if(!_wq_ready(1000)) return 0;

	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = WQ_CMD_WRITE_EN;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;

	_NSS_Port->ODR &= ~_NSS_Pin;
	txbuf[0] = WQ_CMD_CHIP_ERASE;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	_NSS_Port->ODR |= _NSS_Pin;

	if(!_wq_ready(100000)) return 0;
	return 1;
}

uint8_t WQ_Init(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin){
	spi = spiHandle;
	_NSS_Port = NSS_Port;
	_NSS_Pin = NSS_Pin;
	txbuf[0] = 0x90;
	_NSS_Port->ODR &= ~_NSS_Pin;
	HAL_SPI_TransmitReceive(spi, txbuf, rxbuf, 8, 1000);
	_NSS_Port->ODR |= _NSS_Pin;
	if(rxbuf[6] != 0xEF) return 0;
	return 1;
}

