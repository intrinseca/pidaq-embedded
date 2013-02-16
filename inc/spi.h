/*
 * spi.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef SPI_H_
#define SPI_H_

void send_usart(char* string);

void init_spi();
uint8_t send_spi(char* string, uint8_t length);
uint8_t spi_busy(void);
void SPI2_IRQHandler(void);

#endif /* SPI_H_ */
