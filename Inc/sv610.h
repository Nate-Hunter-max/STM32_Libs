/**
 * @file sv610.h
 * @brief SV610 Wireless Transceiver Module Driver for STM32
 * @details Provides interface for configuring and communicating with SV610 modules
 * @version 1.0
 * @date [15.10.2007]
 */

#ifndef SV610_H
#define SV610_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Frequency bands supported by SV610 module
 */
typedef enum {
    SV610_BAND_433MHZ = 1,  ///< 433MHz band (414.92-453.92MHz)
    SV610_BAND_470MHZ = 2,  ///< 470MHz band (470.92-509.92MHz)
    SV610_BAND_868MHZ = 3,  ///< 868MHz band (849.92-888.92MHz)
    SV610_BAND_915MHZ = 4   ///< 915MHz band (895.92-934.92MHz)
} SV610_Band;

/**
 * @brief RF data rate options
 */
typedef enum {
    SV610_RF_RATE_1200 = 0,
    SV610_RF_RATE_2400 = 1,
    SV610_RF_RATE_4800 = 2,
    SV610_RF_RATE_9600 = 3,
    SV610_RF_RATE_14400 = 4,
    SV610_RF_RATE_19200 = 5,
    SV610_RF_RATE_38400 = 6,
    SV610_RF_RATE_57600 = 7,
    SV610_RF_RATE_76800 = 8,
    SV610_RF_RATE_115200 = 9
} SV610_RF_DataRate;

/**
 * @brief RF output power levels
 */
typedef enum {
    SV610_POWER_1DBM = 0,
    SV610_POWER_2DBM = 1,
    SV610_POWER_5DBM = 2,
    SV610_POWER_8DBM = 3,
    SV610_POWER_11DBM = 4,
    SV610_POWER_14DBM = 5,
    SV610_POWER_17DBM = 6,
    SV610_POWER_20DBM = 7
} SV610_TX_Power;

/**
 * @brief Serial parity options
 */
typedef enum {
    SV610_PARITY_NONE = 1,
    SV610_PARITY_ODD = 2,
    SV610_PARITY_EVEN = 3
} SV610_Parity;

/**
 * @brief GPIO pin definition for STM32
 */
typedef struct {
    GPIO_TypeDef* port;  ///< GPIO port (GPIOA, GPIOB, etc.)
    uint16_t pin;        ///< GPIO pin number
} SV610_GPIO;

/**
 * @brief Module configuration structure
 */
typedef struct {
    uint8_t channel;            ///< RF channel (1-40)
    SV610_Band band;            ///< Frequency band
    SV610_RF_DataRate rf_rate;  ///< RF data rate
    SV610_TX_Power power;       ///< TX power level
    uint32_t net_id;            ///< 4-byte network ID
    uint16_t node_id;           ///< 2-byte node ID
    uint32_t serial_rate;       ///< Serial baud rate
    uint8_t data_bits;          ///< Serial data bits (7, 8, or 9)
    uint8_t stop_bits;          ///< Serial stop bits (1 or 2)
    SV610_Parity parity;        ///< Serial parity
} SV610_Config;

/**
 * @brief Module handle structure
 */
typedef struct {
    SV610_Config config;        ///< Current configuration
    bool in_setting_mode;       ///< Flag for setting mode
    UART_HandleTypeDef* huart;  ///< Pointer to HAL UART handle
    SV610_GPIO cs_pin;          ///< CS pin definition
    SV610_GPIO set_pin;         ///< SET pin definition
} SV610_Handle;

/**
 * @brief Initialize SV610 module
 * @param handle Pointer to SV610 handle structure
 * @return true if initialization successful, false otherwise
 */
bool SV610_Init(SV610_Handle* handle);

/**
 * @brief Set module to sleep mode
 * @param handle Pointer to SV610 handle structure
 */
void SV610_Sleep(SV610_Handle* handle);

/**
 * @brief Wake module from sleep mode
 * @param handle Pointer to SV610 handle structure
 */
void SV610_Wake(SV610_Handle* handle);

/**
 * @brief Enter setting mode
 * @param handle Pointer to SV610 handle structure
 * @return true if entered setting mode successfully, false otherwise
 */
bool SV610_EnterSettingMode(SV610_Handle* handle);

/**
 * @brief Exit setting mode
 * @param handle Pointer to SV610 handle structure
 */
void SV610_ExitSettingMode(SV610_Handle* handle);

/**
 * @brief Read current configuration from module
 * @param handle Pointer to SV610 handle structure
 * @return true if configuration read successfully, false otherwise
 */
bool SV610_ReadConfig(SV610_Handle* handle);

/**
 * @brief Write configuration to module
 * @param handle Pointer to SV610 handle structure
 * @return true if configuration written successfully, false otherwise
 */
bool SV610_WriteConfig(SV610_Handle* handle);

/**
 * @brief Reset module to factory defaults
 * @param handle Pointer to SV610 handle structure
 * @return true if reset successful, false otherwise
 */
bool SV610_ResetToDefaults(SV610_Handle* handle);

/**
 * @brief Send data via RF
 * @param handle Pointer to SV610 handle structure
 * @param data Pointer to data buffer
 * @param length Length of data to send
 * @return HAL status (HAL_OK if successful)
 */
HAL_StatusTypeDef SV610_SendData(SV610_Handle* handle, const uint8_t* data, uint16_t length);

/**
 * @brief Check if data is available to receive
 * @param handle Pointer to SV610 handle structure
 * @return Number of bytes available to read
 */
uint16_t SV610_DataAvailable(SV610_Handle* handle);

/**
 * @brief Receive data via RF
 * @param handle Pointer to SV610 handle structure
 * @param buffer Pointer to receive buffer
 * @param max_length Maximum length to receive
 * @return Number of bytes received
 */
uint16_t SV610_ReceiveData(SV610_Handle* handle, uint8_t* buffer, uint16_t max_length);

/**
 * @brief Read RSSI value
 * @param handle Pointer to SV610 handle structure
 * @return RSSI value (0-255) or -1 if error
 */
int16_t SV610_ReadRSSI(SV610_Handle* handle);

#endif // SV610_H
