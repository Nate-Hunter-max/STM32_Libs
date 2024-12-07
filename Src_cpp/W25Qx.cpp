#include "W25Qx.h"

W25Qx::W25Qx(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin, bool softwareNSS) : spi(spiHandle), NSS_Port(
		NSS_Port), NSS_Pin(NSS_Pin), softwareNSS(softwareNSS){
}

uint8_t W25Qx::ready(uint32_t timeout){
	uint32_t begin = HAL_GetTick();
	uint32_t now = HAL_GetTick();
	while((now - begin <= timeout) && (status() & 0x01)) {
		now = HAL_GetTick();
	}
	return (now - begin >= timeout) ? 0 : 1;
}

uint8_t W25Qx::status(){
	txbuf[0] = 0x05;
	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	HAL_SPI_TransmitReceive(spi, txbuf, rxbuf, 2, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;
	return rxbuf[1];
}

uint8_t W25Qx::writePage(uint32_t adr, uint8_t *buf, uint32_t len){
	if(!ready(1000)) return 0;

	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	txbuf[0] = WQ_CMD_WRITE_EN;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;

	txbuf[0] = WQ_CMD_PAGE_PROGRAM;
	txbuf[1] = (uint8_t) (adr >> 16);
	txbuf[2] = (uint8_t) (adr >> 8);
	txbuf[3] = (uint8_t) (adr);
	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	HAL_SPI_Transmit(spi, txbuf, 4, 1000);
	HAL_SPI_Transmit(spi, buf, len, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;
	return 1;
}

uint8_t W25Qx::Write(uint32_t adr, void *buf, uint32_t len){
	uint32_t offset = adr % WQ_PAGE_SIZE;
	uint32_t remaining = len;
	uint32_t to_write;

	while(remaining > 0) {
		to_write = WQ_PAGE_SIZE - offset;
		if(to_write > remaining) {
			to_write = remaining;
		}
		if(!writePage(adr, (uint8_t*) buf, to_write)) {
			return 0;
		}
		adr += to_write;
		buf = (uint8_t*) buf + to_write;
		remaining -= to_write;
		offset = 0;
	}
	return 1;
}

uint8_t W25Qx::Read(uint32_t adr, uint8_t *buf, uint32_t len){
	if(!ready(1000)) return 0;

	txbuf[0] = WQ_CMD_READ;
	txbuf[1] = (uint8_t) (adr >> 16);
	txbuf[2] = (uint8_t) (adr >> 8);
	txbuf[3] = (uint8_t) (adr);
	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	HAL_SPI_Transmit(spi, txbuf, 4, 1000);
	HAL_SPI_Receive(spi, buf, len, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;
	return 1;
}

uint8_t W25Qx::Erase(uint32_t address, uint32_t len){
	uint32_t fSec = address / WQ_SECTOR_SIZE;
	uint32_t lSec = (address + len) / WQ_SECTOR_SIZE;

	for(uint32_t sector = fSec; sector <= lSec; ++sector) {
		if(!ready(1000)) return 0;

		if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
		txbuf[0] = WQ_CMD_WRITE_EN;
		HAL_SPI_Transmit(spi, txbuf, 1, 1000);
		if(softwareNSS) NSS_Port->ODR |= NSS_Pin;

		uint32_t adr = sector * WQ_SECTOR_SIZE;
		txbuf[0] = WQ_CMD_SECTOR_ERASE;
		txbuf[1] = (uint8_t) (adr >> 16);
		txbuf[2] = (uint8_t) (adr >> 8);
		txbuf[3] = (uint8_t) (adr);
		if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
		HAL_SPI_Transmit(spi, txbuf, 4, 1000);
		if(softwareNSS) NSS_Port->ODR |= NSS_Pin;
	}
	return 1;
}

uint8_t W25Qx::ChipErase(){
	if(!ready(1000)) return 0;

	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	txbuf[0] = WQ_CMD_WRITE_EN;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;

	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	txbuf[0] = WQ_CMD_CHIP_ERASE;
	HAL_SPI_Transmit(spi, txbuf, 1, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;

	return ready(100000) ? 1 : 0;
}

uint8_t W25Qx::Init(){
	txbuf[0] = 0x90;
	if(softwareNSS) NSS_Port->ODR &= ~NSS_Pin;
	HAL_SPI_TransmitReceive(spi, txbuf, rxbuf, 8, 1000);
	if(softwareNSS) NSS_Port->ODR |= NSS_Pin;

	return (rxbuf[6] == 0xEF && rxbuf[7] == 0x15) ? 1 : 0;
}
