/**
 * @file sv610.c
 * @brief SV610 Wireless Transceiver Module Implementation for STM32
 * @author Nate Hunter
 * @date [03.05.2024]
 * @details Provides implementation for SV610 module interface using STM32 HAL
 */

#include "sv610.h"
#include <string.h>

// SV610 UART responce buffer
uint8_t response[128] = {0};

// Default configuration
static const SV610_Config DEFAULT_CONFIG = {
    .channel = 20,              // Default channel 20 (433.92MHz for 433MHz band)
    .band = SV610_BAND_433MHZ,  // 433MHz band
    .rf_rate = SV610_RF_RATE_9600,
    .power = SV610_POWER_11DBM,
    .net_id = 0x00000000,
    .node_id = 0x0000,
    .serial_rate = 9600,
    .data_bits = 8,
    .stop_bits = 1,
    .parity = SV610_PARITY_NONE
};

// Default pins configuration
#define DEFAULT_CS_PORT  GPIOB
#define DEFAULT_CS_PIN   GPIO_PIN_13
#define DEFAULT_SET_PORT GPIOB
#define DEFAULT_SET_PIN  GPIO_PIN_14

/**
 * @brief Initialize SV610 module with default pins (PB13 for CS, PB14 for SET)
 * @param handle Pointer to SV610 handle structure
 * @return true if initialization successful, false otherwise
 */
bool SV610_Init(SV610_Handle* handle)
{
    if (!handle || !handle->huart) return false;

    // Set default pins
    handle->cs_pin.port = DEFAULT_CS_PORT;
    handle->cs_pin.pin = DEFAULT_CS_PIN;
    handle->set_pin.port = DEFAULT_SET_PORT;
    handle->set_pin.pin = DEFAULT_SET_PIN;

    handle->in_setting_mode = false;

    // Set default configuration
    memcpy(&handle->config, &DEFAULT_CONFIG, sizeof(SV610_Config));

    // Wake module (CS high)
    HAL_GPIO_WritePin(handle->cs_pin.port, handle->cs_pin.pin, GPIO_PIN_SET);

    // Disable setting mode (SET high)
    HAL_GPIO_WritePin(handle->set_pin.port, handle->set_pin.pin, GPIO_PIN_SET);

    // Wait for module to initialize (2s according to datasheet)
    HAL_Delay(2000);

    return true;
}

void SV610_Sleep(SV610_Handle* handle)
{
    if (!handle) return;
    HAL_GPIO_WritePin(handle->cs_pin.port, handle->cs_pin.pin, GPIO_PIN_RESET);
}

void SV610_Wake(SV610_Handle* handle)
{
    if (!handle) return;
    HAL_GPIO_WritePin(handle->cs_pin.port, handle->cs_pin.pin, GPIO_PIN_SET);
    HAL_Delay(100); // Short delay after wake
}

bool SV610_EnterSettingMode(SV610_Handle* handle)
{
    if (!handle) return false;

    // Must be in working mode to enter setting mode
    HAL_GPIO_WritePin(handle->cs_pin.port, handle->cs_pin.pin, GPIO_PIN_SET);

    // Pull SET low to enter setting mode
    HAL_GPIO_WritePin(handle->set_pin.port, handle->set_pin.pin, GPIO_PIN_RESET);
    HAL_Delay(10);

    // Verify we're in setting mode by reading version
    const uint8_t cmd[] = {0xAA, 0xFA, 0xAA};

    if (HAL_UART_Transmit(handle->huart, cmd, sizeof(cmd), 100) != HAL_OK) return false;

    // Receive up to 32 bytes or until newline
    for (uint8_t i = 0; i < sizeof(response); i++)
    {
        if (HAL_UART_Receive(handle->huart, &response[i], 1, 200) != HAL_OK)
            return false;

        if (response[i] == '\n')
            break;
    }

    handle->in_setting_mode = true;
    return true;
}

void SV610_ExitSettingMode(SV610_Handle* handle)
{
    if (!handle) return;

    // Pull SET high to exit setting mode
    HAL_GPIO_WritePin(handle->set_pin.port, handle->set_pin.pin, GPIO_PIN_SET);
    handle->in_setting_mode = false;
    HAL_Delay(80);
}

bool SV610_ReadConfig(SV610_Handle* handle)
{
    if (!handle || !handle->in_setting_mode) return false;

    const uint8_t cmd[] = {0xAA, 0xFA, 0x01};

    if (HAL_UART_Transmit(handle->huart, cmd, sizeof(cmd), 100) != HAL_OK) return false;
    if (HAL_UART_Receive(handle->huart, response, sizeof(response), 1000) != HAL_OK) return false;

    // Parse response and update handle->config
    // This is a simplified example - actual implementation should properly parse the response
    // ...

    return true;
}

bool SV610_WriteConfig(SV610_Handle* handle)
{
    if (!handle || !handle->in_setting_mode) return false;

    uint8_t cmd[17] = {0xAA, 0xFA, 0x03};

    // Pack configuration into command
    cmd[3] = handle->config.channel;
    cmd[4] = handle->config.band;
    cmd[5] = handle->config.rf_rate;
    cmd[6] = handle->config.power;
    cmd[7] = (handle->config.serial_rate == 115200) ? 9 :
             (handle->config.serial_rate == 76800) ? 8 :
             (handle->config.serial_rate == 57600) ? 7 :
             (handle->config.serial_rate == 38400) ? 6 :
             (handle->config.serial_rate == 19200) ? 5 :
             (handle->config.serial_rate == 14400) ? 4 :
             (handle->config.serial_rate == 9600) ? 3 :
             (handle->config.serial_rate == 4800) ? 2 :
             (handle->config.serial_rate == 2400) ? 1 : 0;

    cmd[8] = handle->config.data_bits - 6; // Maps 7->1, 8->2, 9->3
    cmd[9] = handle->config.stop_bits;
    cmd[10] = handle->config.parity;

    // Net ID (4 bytes)
    cmd[11] = (handle->config.net_id >> 24) & 0xFF;
    cmd[12] = (handle->config.net_id >> 16) & 0xFF;
    cmd[13] = (handle->config.net_id >> 8) & 0xFF;
    cmd[14] = handle->config.net_id & 0xFF;

    // Node ID (2 bytes)
    cmd[15] = (handle->config.node_id >> 8) & 0xFF;
    cmd[16] = handle->config.node_id & 0xFF;

    if (HAL_UART_Transmit(handle->huart, cmd, sizeof(cmd), 100) != HAL_OK) return false;
    if (HAL_UART_Receive(handle->huart, response, 4, 1000) != HAL_OK) return false;

    // Check for "OK" response
    return (response[0] == 'O' && response[1] == 'K');
}

bool SV610_ResetToDefaults(SV610_Handle* handle)
{
    if (!handle || !handle->in_setting_mode) return false;

    const uint8_t cmd[] = {0xAA, 0xFA, 0x02};

    if (HAL_UART_Transmit(handle->huart, cmd, sizeof(cmd), 100) != HAL_OK) return false;
    if (HAL_UART_Receive(handle->huart, response, 4, 1000) != HAL_OK) return false;

    // Check for "OK" response
    if (response[0] == 'O' && response[1] == 'K') {
        memcpy(&handle->config, &DEFAULT_CONFIG, sizeof(SV610_Config));
        return true;
    }

    return false;
}

HAL_StatusTypeDef SV610_SendData(SV610_Handle* handle, const uint8_t* data, uint16_t length)
{
    if (!handle || !data || length == 0) return HAL_ERROR;

    // Must be in working mode (not setting mode)
    if (handle->in_setting_mode) return HAL_ERROR;

    return HAL_UART_Transmit(handle->huart, data, length, HAL_MAX_DELAY);
}

uint16_t SV610_DataAvailable(SV610_Handle* handle)
{
    if (!handle) return 0;

    // Must be in working mode (not setting mode)
    if (handle->in_setting_mode) return 0;

    uint16_t available;
    if (HAL_UART_GetState(handle->huart) == HAL_UART_STATE_READY) {
        HAL_UART_Receive(handle->huart, NULL, 0, 0); // Check available data
        available = handle->huart->RxXferCount;
        return available;
    }
    return 0;
}

uint16_t SV610_ReceiveData(SV610_Handle* handle, uint8_t* buffer, uint16_t max_length)
{
    if (!handle || !buffer || max_length == 0) return 0;

    // Must be in working mode (not setting mode)
    if (handle->in_setting_mode) return 0;

    uint16_t available = SV610_DataAvailable(handle);
    if (available == 0) return 0;

    uint16_t to_read = (available > max_length) ? max_length : available;
    if (HAL_UART_Receive(handle->huart, buffer, to_read, 100) == HAL_OK) {
        return to_read;
    }
    return 0;
}

int16_t SV610_ReadRSSI(SV610_Handle* handle)
{
    if (!handle || !handle->in_setting_mode) return -1;

    const uint8_t cmd[] = {0xAA, 0xFA, 0x04};

    if (HAL_UART_Transmit(handle->huart, cmd, sizeof(cmd), 100) != HAL_OK) return -1;
    if (HAL_UART_Receive(handle->huart, response, 4, 500) != HAL_OK) return -1;

    // Response format: RSSI index\00\r\n (hexadecimal)
    return (int16_t)response[0];
}
