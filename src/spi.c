#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "spi.h"

#define SPI_BUFFER_SIZE 32

uint8_t tx_buffer[SPI_BUFFER_SIZE] = { 0, };
uint8_t tx_offset = 0;
uint8_t tx_length = 0;
uint8_t tx_done = 1;

void init_spi() {
	SPI_InitTypeDef spi_params;
	NVIC_InitTypeDef nvic_params;

	SPI_StructInit(&spi_params);
	SPI_Init(SPI2, &spi_params);

	nvic_params.NVIC_IRQChannel = SPI2_IRQn;
	nvic_params.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_params.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&nvic_params);

	SPI_Cmd(SPI2, ENABLE);
}

uint8_t send_spi(char* string, uint8_t length) {
	if(!tx_done)
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
	tx_done = 0;
	SPI_I2S_SendData(SPI2, tx_buffer[tx_offset++]);
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);

	return 1;
}

void SPI2_IRQHandler(void) {
	if (tx_offset >= tx_length) {
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
		SPI_I2S_SendData(SPI2, 0);
		tx_done = 1;
	}
	else {
		SPI_I2S_SendData(SPI2, tx_buffer[tx_offset++]);
	}
}
