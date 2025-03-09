/**
 * @file tmc2208_driver.h
 * @brief Library for controlling stepper motors using TMC2208 drivers with STM32.
 *
 * This library provides functionality to initialize, configure, and control
 * stepper motors via TMC2208 driver using STEP/DIR and UART interfaces.
 *
 * Created on: 22 January 2025
 * Author: Nate Hunter
 */

#ifndef TMC2208_DRIVER_H
#define TMC2208_DRIVER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Configuration structure for each stepper motor.
 */
typedef struct {
    TIM_HandleTypeDef *htim;        /**< Timer handle for generating STEP pulses. */
    uint32_t tim_channel;          /**< Timer channel for STEP signal. */
    GPIO_TypeDef *dir_port;        /**< GPIO port for DIR pin. */
    uint16_t dir_pin;              /**< GPIO pin for DIR signal. */
    GPIO_TypeDef *enable_port;     /**< GPIO port for EN pin. */
    uint16_t enable_pin;           /**< GPIO pin for EN signal. */
    UART_HandleTypeDef *huart;     /**< UART handle for communication with TMC2208. */
    uint8_t address;               /**< UART address of the TMC2208 driver. */
    uint16_t microsteps;           /**< Microstepping configuration (e.g., 16, 32, 256). */
    uint16_t current_limit;        /**< Current limit for the motor coils in mA. */
} TMC2208_MotorConfig;

/**
 * @brief Initializes a TMC2208 stepper motor.
 *
 * Configures the GPIO, timer, and UART settings required to control the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_Init(TMC2208_MotorConfig *motor);

/**
 * @brief Enables the stepper motor.
 *
 * Activates the motor driver by pulling the EN pin low.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_Enable(TMC2208_MotorConfig *motor);

/**
 * @brief Disables the stepper motor.
 *
 * Deactivates the motor driver by pulling the EN pin high.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_Disable(TMC2208_MotorConfig *motor);

/**
 * @brief Sets the direction of the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 * @param direction Direction of rotation (true for forward, false for backward).
 */
void TMC2208_SetDirection(TMC2208_MotorConfig *motor, bool direction);

/**
 * @brief Sets the speed of the motor.
 *
 * Configures the timer to generate STEP pulses at the specified frequency.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 * @param speed_hz Frequency of STEP pulses in Hz.
 */
void TMC2208_SetSpeed(TMC2208_MotorConfig *motor, uint32_t speed_hz);

/**
 * @brief Configures the TMC2208 driver via UART.
 *
 * Allows setting advanced features like current limit, microstepping resolution,
 * and StealthChop mode.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_ConfigureUART(TMC2208_MotorConfig *motor);

/**
 * @brief Sends a raw UART command to the TMC2208 driver.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 * @param command Pointer to the command buffer.
 * @param length Length of the command buffer.
 */
void TMC2208_SendUARTCommand(TMC2208_MotorConfig *motor, uint8_t *command, uint8_t length);

/**
 * @brief Starts generating STEP pulses for the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_StartSteps(TMC2208_MotorConfig *motor);

/**
 * @brief Stops generating STEP pulses for the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_StopSteps(TMC2208_MotorConfig *motor);

#endif // TMC2208_DRIVER_H
