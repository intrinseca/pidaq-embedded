/*
 * spi.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef SPI_H_
#define SPI_H_

void send_usart(char* string);

extern volatile uint8_t spi_tx_done;

void spi_init();
uint8_t spi_send_string(const char* string, uint8_t length);
uint8_t spi_busy(void);
void SPI2_IRQHandler(void);

#endif /* SPI_H_ */
