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
		string++;
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE ) == RESET)
			;
	} while (*string);
}

void spi_init() {
	SPI_InitTypeDef spi_params;
	NVIC_InitTypeDef nvic_params;

	//Enable peripherals
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//Configure SPI
	SPI_StructInit(&spi_params);
	SPI_Init(SPI2, &spi_params);

	//Configure SPI Interrupts
	nvic_params.NVIC_IRQChannel = SPI2_IRQn;
	nvic_params.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_params.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&nvic_params);

	//Configure DMA Interrupts
	nvic_params.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	nvic_params.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_params.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&nvic_params);

	//Start sending 0 bytes
	spi_tx_done = 1;
	SPI_I2S_SendData(SPI2, 0x00);
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);

	//Enable the SPI Bus
	SPI_Cmd(SPI2, ENABLE);
}

uint8_t spi_send_string(const char* string, uint8_t length) {
	DMA_InitTypeDef dma_params;

	if (!spi_tx_done)
		//Error, previous transfer not complete
		return 0;

	//Copy the data to the SPI buffer
	tx_offset = 0;
	tx_length = length + 1;

	tx_buffer[tx_offset++] = length;

	while (length > 0) {
		tx_buffer[tx_offset++] = *string;
		string++;
		length--;
	}

	//Configure the DMA channel
	DMA_DeInit(DMA1_Channel5 );
	DMA_StructInit(&dma_params);
	dma_params.DMA_PeripheralBaseAddr = (uint32_t) (&(SPI2 ->DR));
	dma_params.DMA_MemoryBaseAddr = (uint32_t) tx_buffer;
	dma_params.DMA_DIR = DMA_DIR_PeripheralDST;
	dma_params.DMA_BufferSize = tx_length;
	dma_params.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_params.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_Init(DMA1_Channel5, &dma_params);

	//Enable DMA complete interrupt
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

	//Turn off TXE interrupt to stop 0-filling
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

	//Enable the DMA flag on the SPI
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

	//Enable the DMA controller
	DMA_Cmd(DMA1_Channel5, ENABLE);

	//Transfer is officially in progress
	spi_tx_done = 0;

	//Return sucess
	return 1;
}

void DMA1_Channel5_IRQHandler(void) {
	//All data transferred from buffer, can now refill
	spi_tx_done = 1;

	//Turn of the DMA
	DMA_Cmd(DMA1_Channel5, DISABLE);
	DMA_ClearITPendingBit(DMA1_IT_TC5);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, DISABLE);

	//Start 0-filling
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
}

void SPI2_IRQHandler(void) {
	//Zero-fill if no transfer in progress
	if (spi_tx_done) {
		SPI_I2S_SendData(SPI2, 0x00);
	}
}
