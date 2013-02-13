/*
 * pool.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef POOL_H_
#define POOL_H_

#define POOL_BUFF_SIZE  8

void * pool_malloc_buff(void);
void pool_free_buff(void * handle);
void pool_init(void);

#endif /* POOL_H_ */
