/**
 * @file WS2812B.c
 * @brief Library for controlling WS2812B LED strips using STM32.
 *
 *  Created on: Dec 28, 2024
 *      Author: Nate Hunter
 *
 * This library provides functions for initializing, updating, and controlling WS2812B LED strips.
 */

#include "WS2812B.h"
#include <stdlib.h>
#include <string.h>
/**
 * @brief Initializes the WS2812B LED strip.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @return 1 on success, 0 on failure.
 */
uint8_t WS2812B_Init(WS2812B_Strip *strip) {
    if (strip == NULL) {
        return 0;
    }
    strip->buffer_length = strip->led_count * 24 + WS2812B_DELAY_LEN;

    strip->dma_buffer = (uint32_t *)malloc(strip->buffer_length * sizeof(uint32_t));
    if (!strip->dma_buffer) {
        return 0;
    }

  	uint32_t i;
  	for(i = 0; i < strip->buffer_length - WS2812B_DELAY_LEN; i++) {
  		strip->dma_buffer[i] = WS2812B_LOW_P;
  	}
  	for(; i < strip->buffer_length; i++) {
  		strip->dma_buffer[i] = 0;
  	}
  	RGB_Color rgb = {.r=255,.g=255,.b=255};
  	WS2812B_Fill(strip, &rgb);
  	HAL_StatusTypeDef res = HAL_TIM_PWM_Start_DMA(strip->tim, strip->channel, strip->dma_buffer, strip->buffer_length);
  	HAL_Delay(1);
    return 1;
}

/**
 * @brief Updates the frame data to the LED strip via DMA.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 */
void WS2812B_UpdateFrame(WS2812B_Strip *strip) {
    HAL_TIM_PWM_Start_DMA(strip->tim, strip->channel, strip->dma_buffer, strip->buffer_length);
}

/**
 * @brief Sets the color of a specific LED.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @param pos LED position in the strip (0-based index).
 * @param color Pointer to a RGB_Color structure containing RGB values.
 */
void WS2812B_SetColor(WS2812B_Strip *strip, uint16_t pos, RGB_Color *color) {
    if (pos >= strip->led_count || !color) {
        return;
    }

    for (uint8_t j = 0; j < 8; j++) {
        strip->dma_buffer[pos * 24 + j + 0x00] = (color->g & (1 << (7 - j))) ? WS2812B_HIGH_P : WS2812B_LOW_P;
        strip->dma_buffer[pos * 24 + j + 0x08] = (color->r & (1 << (7 - j))) ? WS2812B_HIGH_P : WS2812B_LOW_P;
        strip->dma_buffer[pos * 24 + j + 0x10] = (color->b & (1 << (7 - j))) ? WS2812B_HIGH_P : WS2812B_LOW_P;
    }
}

/**
 * @brief Fills the entire LED strip with a single color.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @param color Pointer to a RGB_Color structure containing RGB values.
 */
void WS2812B_Fill(WS2812B_Strip *strip, RGB_Color *color) {
    if (!color) {
        return;
    }

    for (uint16_t k = 0; k < strip->led_count; k++) {
        WS2812B_SetColor(strip, k, color);
    }
}

/**
 * @brief Fills the entire LED strip with colors from an array.
 *
 * @param strip Pointer to the WS2812B_Strip structure.
 * @param colors Pointer to an array of RGB_Color structures.
 */
void WS2812B_FillArray(WS2812B_Strip *strip, RGB_Color *colors) {
    if (!colors) {
        return;
    }

    for (uint16_t k = 0; k < strip->led_count; k++) {
        WS2812B_SetColor(strip, k, &colors[k]);
    }
}

