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

class W25Qx {
public:
    W25Qx(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *NSS_Port, uint16_t NSS_Pin, bool softwareNSS);
    uint8_t Write(uint32_t adr, void *buf, uint32_t len);
    uint8_t Read(uint32_t adr, uint8_t *buf, uint32_t len);
    uint8_t Erase(uint32_t address, uint32_t len);
    uint8_t ChipErase();
    uint8_t Init();
private:
    SPI_HandleTypeDef *spi;
    uint8_t txbuf[10];
    uint8_t rxbuf[10];
    GPIO_TypeDef *NSS_Port;
    uint16_t NSS_Pin;
    uint8_t softwareNSS;

    uint8_t ready(uint32_t timeout);
    uint8_t writePage(uint32_t adr, uint8_t *buf, uint32_t len);
    uint8_t status();
};
