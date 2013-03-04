/*
 * spi.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef SPI_H_
#define SPI_H_

#include "pool.h"

void send_usart(char* string);

extern volatile uint8_t spi_tx_done;

void spi_init();
uint8_t spi_send_string(const pool_item_t * string, uint8_t length);
uint8_t spi_busy(void);
void SPI2_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);

#endif /* SPI_H_ */
