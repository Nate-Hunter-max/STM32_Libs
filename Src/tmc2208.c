/**
 * @file tmc2208_driver_impl.c
 * @brief Implementation of TMC2208 stepper motor control functions.
 *
 * This file contains the implementation of functions for controlling stepper motors using the TMC2208 driver.
 * It supports STEP/DIR interface and UART configuration.
 *
 * Created on: 22 January 2025
 * Author: Nate Hunter
 */

#include "tmc2208.h"

/**
 * @brief Initializes a TMC2208 stepper motor.
 *
 * Configures the timer and UART settings required to control the motor.
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_Init(TMC2208_MotorConfig *motor) {
    // Set initial states
    HAL_GPIO_WritePin(motor->enable_port, motor->enable_pin, GPIO_PIN_SET); // Disable motor by default
    HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);    // Default direction

    // Configure timer for STEP pulses
    TIM_OC_InitTypeDef sConfigOC = {0};

    motor->htim->Init.Prescaler = 0;
    motor->htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    motor->htim->Init.Period = 1000; // Default period (adjust later for speed)
    motor->htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    motor->htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(motor->htim);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 500; // Default pulse width (50% duty cycle)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(motor->htim, &sConfigOC, motor->tim_channel);

    // Enable UART if provided
    if (motor->huart) {
        TMC2208_ConfigureUART(motor);
    }
}

/**
 * @brief Enables the stepper motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_Enable(TMC2208_MotorConfig *motor) {
    HAL_GPIO_WritePin(motor->enable_port, motor->enable_pin, GPIO_PIN_RESET); // Enable motor
}

/**
 * @brief Disables the stepper motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_Disable(TMC2208_MotorConfig *motor) {
    HAL_GPIO_WritePin(motor->enable_port, motor->enable_pin, GPIO_PIN_SET); // Disable motor
}

/**
 * @brief Sets the direction of the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 * @param direction Direction of rotation (true for forward, false for backward).
 */
void TMC2208_SetDirection(TMC2208_MotorConfig *motor, bool direction) {
    HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, direction ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Sets the speed of the motor.
 *
 * Configures the timer to generate STEP pulses at the specified frequency.
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 * @param speed_hz Frequency of STEP pulses in Hz.
 */
void TMC2208_SetSpeed(TMC2208_MotorConfig *motor, uint32_t speed_hz) {
    uint32_t timer_clk = HAL_RCC_GetPCLK1Freq();
    uint32_t period = timer_clk / speed_hz;

    __HAL_TIM_SET_AUTORELOAD(motor->htim, period - 1);
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, period / 2); // 50% duty cycle

    HAL_TIM_PWM_Start(motor->htim, motor->tim_channel);
}

/**
 * @brief Starts generating STEP pulses for the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_StartSteps(TMC2208_MotorConfig *motor) {
    HAL_TIM_PWM_Start(motor->htim, motor->tim_channel);
}

/**
 * @brief Stops generating STEP pulses for the motor.
 *
 * @param motor Pointer to the TMC2208_MotorConfig structure.
 */
void TMC2208_StopSteps(TMC2208_MotorConfig *motor) {
    HAL_TIM_PWM_Stop(motor->htim, motor->tim_channel);
}

//TODO: swuart_calcCRC is from datasheet. Review.
void swuart_calcCRC(uint8_t *datagram, uint8_t datagramLength) {
	int i, j;
	uint8_t *crc = datagram + (datagramLength - 1); // CRC located in last byte of message
	uint8_t currentByte;
	*crc = 0;
	for (i = 0; i < (datagramLength - 1); i++) { // Execute for all bytes of a message
		currentByte = datagram[i]; // Retrieve a byte to be sent from Array
		for (j = 0; j < 8; j++) {
			if ((*crc >> 7) ^ (currentByte & 0x01)) // update CRC based result of XOR operation
					{
				*crc = (*crc << 1) ^ 0x07;
			} else {
				*crc = (*crc << 1);
			}
			currentByte = currentByte >> 1;
		} // for CRC bit
	} // for message byte
}

//TODO: Add uart CMD in lib. TMC2208_Send & TMC2208_Receive are from tests.
// Test data: uint8_t command[] = { 0x05, 0x00, 0x06, 0xFF };
void TMC2208_Send(uint8_t *data, uint8_t length) {
	swuart_calcCRC(data, 4);
	HAL_HalfDuplex_EnableTransmitter(&huart2);
	HAL_UART_Transmit(&huart2, data, length,1000);
	HAL_HalfDuplex_EnableReceiver(&huart2);
}

void TMC2208_Receive(uint8_t *buffer, uint8_t length) {
	HAL_UART_Receive(&huart2, buffer, length, 1000);
}
