/**
 * @file ws2812b.h
 * @brief Header file for the WS2812B LED strip library.
 *
 *  Created on: Dec 28, 2024
 *      Author: Nate Hunter
 *
 * This file defines the structures, constants, and function prototypes
 * for controlling WS2812B LED strips.
 */

#ifndef WS2812B_H
#define WS2812B_H

#include "main.h"

// Configuration macros
#define WS2812B_HIGH_P 67       ///< High pulse duration for WS2812B
#define WS2812B_LOW_P  34       ///< Low pulse duration for WS2812B
#define WS2812B_DELAY_LEN 50    ///< Reset delay for WS2812B

// Type definitions

/**
 * @brief Structure for representing an RGB color.
 */
typedef struct {
	uint8_t r; ///< Red intensity (0-255)
	uint8_t g; ///< Green intensity (0-255)
	uint8_t b; ///< Blue intensity (0-255)
} RGB_Color;

/**
 * @brief Structure for managing an WS2812B LED strip.
 */
typedef struct {
	TIM_HandleTypeDef *tim; ///< Timer handle for PWM
	uint32_t channel;       ///< Timer PWM channel
	uint16_t led_count;     ///< Number of LEDs in the strip
	uint32_t buffer_length; ///< Length of the DMA buffer
	uint32_t *dma_buffer;   ///< Pointer to the DMA buffer
} WS2812B_Strip;

// Function prototypes

/**
 * @brief Initializes the WS2812B LED strip.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @return 1 on success, 0 on failure.
 */
uint8_t WS2812B_Init(WS2812B_Strip *strip);

/**
 * @brief Updates the frame data to the LED strip via DMA.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 */
void WS2812B_UpdateFrame(WS2812B_Strip *strip);

/**
 * @brief Sets the color of a specific LED.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @param pos LED position in the strip (0-based index).
 * @param color Pointer to a RGB_Color structure containing RGB values.
 */
void WS2812B_SetColor(WS2812B_Strip *strip, uint16_t pos, RGB_Color *color);

/**
 * @brief Fills the entire LED strip with a single color.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @param color Pointer to a RGB_Color structure containing RGB values.
 */
void WS2812B_Fill(WS2812B_Strip *strip, RGB_Color *color);

/**
 * @brief Fills the entire LED strip with colors from an array.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @param colors Pointer to an array of RGB_Color structures.
 */
void WS2812B_FillArray(WS2812B_Strip *strip, RGB_Color *colors);

#endif // WS2812B_H

