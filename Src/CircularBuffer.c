/*
 * CircularBuffer.c
 *
 *  Created on: Mar 9, 2025
 *      Author: Nate Hunter
 */

#include "CircularBuffer.h"

CircularBuffer* CB_Create(uint8_t size){
	CircularBuffer *buffer = (CircularBuffer*) malloc(sizeof(CircularBuffer));
	buffer->data = (uint32_t*) malloc(size * sizeof(uint32_t));
	for(uint8_t i = 0; i < size; i++)
		buffer->data[i] = 0;
	buffer->size = size;
	buffer->head = 0;
	buffer->tail = 0;
	return buffer;
}

void CB_Free(CircularBuffer *buffer){
	free(buffer->data);
	free(buffer);
}

void CB_Add(CircularBuffer *buffer, int measurement){
	buffer->data[buffer->tail] = measurement;
	buffer->tail = (buffer->tail + 1) % buffer->size;
	if(buffer->tail == buffer->head) {
		buffer->head = (buffer->head + 1) % buffer->size;
	}
}

uint32_t CB_Diff(CircularBuffer *buffer){
	uint32_t maxDiff = 0;
	if(buffer->head != buffer->tail) {
		uint8_t i = buffer->head;
		uint8_t j = (buffer->head + 1) % buffer->size;
		do {
			uint32_t diff = abs(buffer->data[i] - buffer->data[j]);
			if(diff > maxDiff) {
				maxDiff = diff;
			}
			i = (i + 1) % buffer->size;
			j = (j + 1) % buffer->size;
		} while(i != buffer->tail);
	}
	return maxDiff;
}
