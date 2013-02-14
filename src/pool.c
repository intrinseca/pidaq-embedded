/*
 * pool.c
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#include "pool.h"
#include <string.h>

#define NUM_BUFFERS 8

enum ALLOCED_STATUS {
	BUFF_FREE, BUFF_NOT_FREE
};

unsigned char pool[NUM_BUFFERS][POOL_BUFF_SIZE];

unsigned char alloced[NUM_BUFFERS];

/*
 * Initialise the memory pool. Set all buffers to be 'free' and zero out all
 * arrays.
 */
void pool_init(void) {
	unsigned char i, j;

	for (i = 0; i < NUM_BUFFERS; ++i) {
		for(j = 0; j < POOL_BUFF_SIZE; ++j)
		{
			pool[i][j] = 0;
		}
		alloced[i] = BUFF_FREE;
	}
}

/*
 * Retrieves a new buffer from the pool. Returns the address of the new buffer,
 * or NULL if a buffer failed to be allocated.
 */
void * pool_malloc_buff(void) {
	unsigned char i;

	/* try and find an unallocated buffer */
	for (i = 0; i < NUM_BUFFERS; ++i) {
		if (alloced[i] == BUFF_FREE) {
			alloced[i] = BUFF_NOT_FREE;
			return pool[i];
		}
	}

	return 0; // all buffers allocated
}

void pool_free_buff(void * handle) {
	unsigned char i;

	for (i = 0; i < NUM_BUFFERS; ++i) {
		if (pool[i] == handle) {
			alloced[i] = BUFF_FREE;
			return;
		}
	}

	return; // FIXME: check: we should never get here.
}