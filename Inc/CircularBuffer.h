#pragma once
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Circular buffer structure for generic data types.
 */
typedef struct {
	void *data;            /**< Pointer to the buffer memory. */
	uint8_t size;          /**< Maximum number of elements in the buffer. */
	uint8_t item_size;   	/**< Size of a single element in bytes. */
	uint8_t head;          /**< Index of the oldest element. */
	uint8_t tail;          /**< Index of the next free position. */
} CircularBuffer;

/**
 * @brief Create a circular buffer.
 *
 * @param buffer Pointer to buffer structure.
 */
void CB_Init(CircularBuffer *buffer);

/**
 * @brief Free the memory allocated for the circular buffer.
 *
 * @param buffer Pointer to the buffer.
 */
void CB_Free(CircularBuffer *buffer);

/**
 * @brief Add an item to the circular buffer.
 *
 * @param buffer Pointer to the buffer.
 * @param item Pointer to the item to add.
 */
void CB_Add(CircularBuffer *buffer, const void *item);

/**
 * @brief Calculate the maximum difference between consecutive elements.
 *
 * @param buffer Pointer to the buffer.
 * @param compare Function pointer for comparison logic.
 * @return uint32_t Maximum difference found.
 */
uint32_t CB_Diff(CircularBuffer *buffer, uint32_t (*compare)(const void*, const void*));

/**
 * @brief Calculate the average value of all elements in the buffer.
 *
 * @param buffer Pointer to the buffer.
 * @param sum Function pointer to sum two elements.
 * @param divide Function pointer to divide the sum by the count.
 * @return uint32_t Average value of the elements.
 */
uint32_t CB_Average(CircularBuffer *buffer, uint32_t (*sum)(const void*, const void*), uint32_t (*divide)(const void*, uint32_t));
