/*
 * CircularBuffer.h
 *
 *  Created on: May 10, 2024
 *      Author: Nate Hunter
 */

#pragma once
#include <stdlib.h>
#include <stdint.h>
typedef struct {
	uint32_t *data;
	uint8_t size;
	uint8_t head;
	uint8_t tail;
} CircularBuffer;

CircularBuffer* CB_Create(uint8_t size);

void CB_Free(CircularBuffer *buffer);

void CB_Add(CircularBuffer *buffer, int measurement);

uint32_t CB_Diff(CircularBuffer *buffer);
