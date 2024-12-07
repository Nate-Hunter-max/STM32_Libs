/*
 * WQ.h
 *
 *  Created on: Jun 9, 2024
 *      Author: Nate Hunter
 */

#pragma once
#include "main.h"

#define WQ_SECTOR_SIZE 0x1000
#define WQ_PAGE_SIZE 256
#define WQ_CMD_WRITE_EN 0x06
#define WQ_CMD_WRITE_DIS 0x04
#define WQ_CMD_SECTOR_ERASE 0x20
#define WQ_CMD_READ 0x03
#define WQ_CMD_PAGE_PROGRAM 0x02
#define WQ_CMD_CHIP_ERASE 0xC7

uint8_t WQ_Init(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin);
uint8_t WQ_Write(uint32_t adr, void *buf, uint32_t len);
uint8_t WQ_Read(uint32_t adr, uint8_t *buf, uint32_t len);
uint8_t WQ_Erace(uint32_t address, uint32_t len);
uint8_t WQ_Chip_Erace();
