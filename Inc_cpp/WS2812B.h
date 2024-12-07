/*
 * WS2812B.h
 *
 *  Created on: Jul 8, 2024
 *      Author: Nate Hunter
 */

#pragma once

#include "main.h"

#define HIGH_P 67
#define LOW_P 34
#define DELAY_LEN 50

#define LED_CHANNEL_1 TIM_CHANNEL_3
#define LED_CHANNEL_2 TIM_CHANNEL_2
#define LED_CHANNEL_3 TIM_CHANNEL_1

#define RGB_RED 255,0,0
#define RGB_GREEN 0,255,0
#define RGB_BLUE 0,0,255

#define RGB_CYAN 0,255,255
#define RGB_MAGENTA 255,0,255
#define RGB_YELLOW 255,255,0

#define RGB_BLACK 0,0,0
#define RGB_WHITE 255,255,255

class LedStrip {
public:
    LedStrip(TIM_HandleTypeDef *tim, uint8_t channel, uint16_t ledCnt);
    void init();
    void updateFrame();
    void setColor(uint8_t pos, uint8_t r, uint8_t g, uint8_t b);
    void fill(uint8_t r, uint8_t g, uint8_t b);
    void fill(uint8_t* rgb);
private:
    uint8_t _channel;
    uint32_t _bufLen;
    uint16_t _ledCnt;
    uint32_t *DMA_Buffer;
    TIM_HandleTypeDef *_tim;
};



