/*
 * WS2812B.cpp
 *
 *  Created on: Jul 9, 2024
 *      Author: Nate Hunter
 */

#include "WS2812B.h"
#include <stdlib.h>
LedStrip::LedStrip(TIM_HandleTypeDef *tim, uint8_t channel, uint16_t ledCnt){
	_channel = channel;
	_ledCnt = ledCnt;
	_bufLen = _ledCnt * 24 + DELAY_LEN;
	DMA_Buffer = (uint32_t*) malloc(_bufLen * sizeof(uint32_t));
	_tim = tim;
	init();
}

void LedStrip::init(){
	uint32_t i;
	for(i = 0; i < _bufLen - DELAY_LEN; i++) {
		DMA_Buffer[i] = LOW_P;
	}
	for(; i < _bufLen; i++) {
		DMA_Buffer[i] = 0;
	}
}

void LedStrip::updateFrame(){
	HAL_TIM_PWM_Start_DMA(_tim, _channel, DMA_Buffer, _bufLen);
}

void LedStrip::setColor(uint8_t pos, uint8_t r, uint8_t g, uint8_t b){
	for(uint8_t j = 0; j < 8; j++) {
		DMA_Buffer[pos * 24 + j + 0x00] = (g & (1 << (7 - j))) ? HIGH_P : LOW_P;
		DMA_Buffer[pos * 24 + j + 0x08] = (r & (1 << (7 - j))) ? HIGH_P : LOW_P;
		DMA_Buffer[pos * 24 + j + 0x10] = (b & (1 << (7 - j))) ? HIGH_P : LOW_P;
	}
}

void LedStrip::fill(uint8_t r, uint8_t g, uint8_t b){
	for(uint8_t k = 0; k < _ledCnt; k++) {
		setColor(k, r, g, b);
	}
}
void LedStrip::fill(uint8_t *rgb){
	for(uint8_t k = 0; k < _ledCnt; k++) {
		setColor(k, rgb[0],rgb[1], rgb[2]);
	}
}

