/*
 * pool.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef POOL_H_
#define POOL_H_

#define POOL_BUFF_SIZE  200
#define POOL_NUM_BUFFERS 8

void * pool_malloc_buff(void);
void pool_free_buff(void * handle);
void pool_init(void);

extern unsigned char alloced_num;

#endif /* POOL_H_ */
