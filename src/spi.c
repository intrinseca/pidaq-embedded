#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "spi.h"
#include "pool.h"

#define SPI_BUFFER_SIZE 255

uint8_t tx_buffer[SPI_BUFFER_SIZE] = { 0, };
uint8_t tx_offset = 0;
uint8_t tx_length = 0;
volatile uint8_t spi_tx_done = 1;

void send_usart(char* string) {
	do {
		USART_SendData(USART1, *string);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE ) == RESET)
			;
	} while (*string++);
}

void spi_init() {
	SPI_InitTypeDef spi_params;
	NVIC_InitTypeDef nvic_params;

	SPI_StructInit(&spi_params);
	SPI_Init(SPI2, &spi_params);

	nvic_params.NVIC_IRQChannel = SPI2_IRQn;
	nvic_params.NVIC_IRQChannelPreemptionPriority = 1;
	nvic_params.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&nvic_params);

	spi_tx_done = 1;
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
	SPI_I2S_SendData(SPI2, 0x00);

	SPI_Cmd(SPI2, ENABLE);
}

uint8_t spi_send_string(const char* string, uint8_t length) {
	if (!spi_tx_done)
		return 0;

	tx_offset = 0;
	tx_length = length + 1;

	tx_buffer[tx_offset++] = length;

	while (length > 0) {
		tx_buffer[tx_offset++] = *string;
		string++;
		length--;
	}

	tx_offset = 0;
	spi_tx_done = 0;

	return 1;
}

void SPI2_IRQHandler(void) {
	if(spi_tx_done) {
		SPI_I2S_SendData(SPI2, 0x00);
	}
	else {
		SPI_I2S_SendData(SPI2, tx_buffer[tx_offset++]);
		if(tx_offset >= tx_length)
			spi_tx_done = 1;
	}
}
