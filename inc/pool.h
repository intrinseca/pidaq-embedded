/*
 * pool.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef POOL_H_
#define POOL_H_

//POOL_BUFF_SIZE must be a multiple of four + 1 so that the interleaving works!
#define POOL_BUFF_SIZE  201
#define POOL_NUM_BUFFERS 8

typedef uint16_t pool_item_t;

pool_item_t * pool_malloc_buff(void);
void pool_free_buff(pool_item_t * handle);
void pool_init(void);

extern unsigned char alloced_num;

#endif /* POOL_H_ */
