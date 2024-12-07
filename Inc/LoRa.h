/*
 * LoRa.h
 *
 *  Created on: Nov 29, 2024
 *      Author: Nate Hunter
 */
#pragma once
#include "main.h"

#define LORA_Fifo                0x00
#define LORA_RegOpMode        	 0x01
#define LORA_RegFrMsd            0x06
#define LORA_RegFrMid            0x07
#define LORA_RegFrLsd            0x08
#define LORA_RegPaConfig         0x09
#define LORA_RegFifoAddrSpi      0x0D
#define LORA_RegFifoTxBaseAddr   0x0E
#define LORA_RegFifoRxBaseAddr   0x0F
#define LORA_RegLna              0x0C
#define LORA_RegFifoRxBaseAddr   0x0F
#define LORA_RegFifoRxCurrentAdr 0x10
#define LORA_RegIrqFlagsMask     0x11
#define LORA_RegIrqFlags         0x12
#define LORA_RegRxNbBytes        0x13
#define LORA_RegModemConfig1     0x1D
#define LORA_RegModemConfig2     0x1E
#define LORA_RegPreambleMsd      0x20
#define LORA_RegPreambleLsd      0x21
#define LORA_RegPayloadLenght    0x22
#define LORA_RegPayloadMaxLenght 0x23
#define LORA_RegFifoRxByteAddr   0x25
#define LORA_RegModemConfig3	    0x26
#define LORA_RegVersion          0x42

#define LORA_Flag_CadDetected         0x01
#define LORA_Flag_FhssChangeChannel   0x02
#define LORA_Flag_CadDone            0x04
#define LORA_Flag_TxDone             0x08
#define LORA_Flag_ValidHeader         0x10
#define LORA_Flag_PayloadCrcError     0x20
#define LORA_Flag_RxDone            0x40
#define LORA_Flag_RxTimeout           0x80

#define LORA_WRITE_BIT          (1<<7)
#define LORA_READ_BIT 					0x76

typedef struct {
    uint32_t frequency;          // В частоте, например, 433000000 для 433 МГц
    uint8_t bandwidth;           // 7.8 | 10.4 | 15.6 | ... | 500 kHz, значения: 0 - 9
    uint8_t spreadingFactor;     // 6 | 7 | 8 | ... | 12, значения: 6 - 12
    uint8_t codingRate;          // 4/5 | 4/6 | 4/7 | 4/8, значения: 0 - 3
    uint8_t headerMode;          // 1 = fixed header, 0 = explicit header
    uint8_t crcEnabled;          // 0 = CRC отключён, 1 = CRC включён
    uint8_t lowDataRateOptimize; // 0 = отключено, 1 = включено
    uint8_t preambleLength;      // Минимальное значение: 4
    uint8_t payloadLength;       // Максимальное значение: 255
    uint8_t txPower;             // Мощность передачи: 0 - 15
    uint8_t txAddr;              // Базовый адрес FIFO для передачи: 0 - 255
    uint8_t rxAddr;              // Базовый адрес FIFO для приёма: 0 - 255
} LoRaConfig;

typedef struct {
    SPI_HandleTypeDef *spi;
    GPIO_TypeDef *NSS_Port;
    uint16_t NSS_Pin;
    LoRaConfig config;
} LoRa_HandleTypeDef;

uint8_t LoRa_Init(LoRa_HandleTypeDef *handle);
void LoRa_SetConfig(LoRa_HandleTypeDef *handle, LoRaConfig *config);
void LoRa_Transmit(LoRa_HandleTypeDef *handle, void *data, uint8_t len);
uint8_t LoRa_Receive(LoRa_HandleTypeDef *handle, void *rxData, uint8_t *len);
