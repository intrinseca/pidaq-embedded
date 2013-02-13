/*
 * spi.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef SPI_H_
#define SPI_H_

void init_spi();
uint8_t send_spi(char* string, uint8_t length);
void SPI2_IRQHandler(void);

#endif /* SPI_H_ */
