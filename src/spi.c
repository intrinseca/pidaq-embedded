#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "spi.h"
#include "pool.h"

#define SPI_BUFFER_SIZE 255

uint8_t tx_buffer[SPI_BUFFER_SIZE] = { 0, };
uint8_t tx_offset = 0;
uint8_t tx_length = 0;
volatile uint8_t spi_tx_done = 1;
uint8_t tx_fill = 0;

DMA_InitTypeDef dma_params;

void spi_zero_fill();

void send_usart(char* string) {
    do {
        USART_SendData(USART1, *string);
        string++;
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
    } while (*string);
}

void spi_init() {
    SPI_InitTypeDef spi_params;
    NVIC_InitTypeDef nvic_params;

    //Enable peripherals
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    //Configure SPI
    SPI_StructInit(&spi_params);
    SPI_Init(SPI2, &spi_params);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

    //Configure SPI Interrupts
    nvic_params.NVIC_IRQChannel = SPI2_IRQn;
    nvic_params.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_params.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvic_params);

    //Base Configuration for DMA
    DMA_StructInit(&dma_params);
    dma_params.DMA_PeripheralBaseAddr = (uint32_t) (&(SPI2 ->DR));
    dma_params.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_params.DMA_Priority = DMA_Priority_VeryHigh;

    //Configure DMA Interrupts
    nvic_params.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    nvic_params.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_params.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic_params);

    //Enable the SPI Bus
    SPI_Cmd(SPI2, ENABLE);

    //Start sending 0 bytes
    spi_tx_done = 1;
    spi_zero_fill();
}

void spi_zero_fill() {
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);

    dma_params.DMA_MemoryBaseAddr = (uint32_t) &tx_fill;
    dma_params.DMA_BufferSize = 1;
    dma_params.DMA_MemoryInc = DMA_MemoryInc_Disable;
    dma_params.DMA_Mode = DMA_Mode_Circular;
    DMA_Init(DMA1_Channel5, &dma_params);

    DMA_Cmd(DMA1_Channel5, ENABLE);
}

uint8_t spi_send_string(const char* string, uint8_t length) {
    if (!spi_tx_done) {
        //Error, previous transfer not complete
        return 0;
    }

    //Configure the DMA channel
    DMA_Cmd(DMA1_Channel5, DISABLE);
    dma_params.DMA_MemoryBaseAddr = (uint32_t) string;
    dma_params.DMA_BufferSize = length;
    dma_params.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_params.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel5, &dma_params);

    //Enable DMA complete interrupt
    DMA_ClearITPendingBit(DMA1_IT_TC5);
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

    //Enable the DMA controller
    DMA_Cmd(DMA1_Channel5, ENABLE);

    //Transfer is officially in progress
    spi_tx_done = 0;

    //Return success
    return 1;
}

void DMA1_Channel5_IRQHandler(void) {
    //All data transferred from buffer, can now refill
    spi_tx_done = 1;

    //Start zero filling
    spi_zero_fill();

    //Clear Interrupt
    DMA_ClearITPendingBit(DMA1_IT_TC5);
}
