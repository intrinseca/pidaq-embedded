#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "spi.h"
#include "pool.h"
#include "gpio.h"

#define SPI_BUFFER_SIZE 255

volatile uint8_t spi_tx_done = 1;
pool_item_t tx_fill = 0;

DMA_InitTypeDef dma_params;

void spi_zero_fill();

void spi_init() {
    SPI_InitTypeDef spi_params;
    NVIC_InitTypeDef nvic_params;

    //Enable peripherals
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    //Configure SPI
    SPI_StructInit(&spi_params);
    spi_params.SPI_DataSize = SPI_DataSize_16b;
    SPI_Init(SPI2, &spi_params);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

    //Configure SPI Interrupts
    nvic_params.NVIC_IRQChannel = SPI2_IRQn;
    nvic_params.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_params.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvic_params);
    SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);

    //Base Configuration for DMA
    DMA_StructInit(&dma_params);
    dma_params.DMA_PeripheralBaseAddr = (uint32_t) (&(SPI2 ->DR));
    dma_params.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_params.DMA_Priority = DMA_Priority_VeryHigh;
    dma_params.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_params.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

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

uint8_t spi_send_string(pool_item_t * string, uint8_t length) {
    char data;

    if (!spi_tx_done) {
        //Error, previous transfer not complete
        return 0;
    }

    //Load the Digital I/O Input into the top part of the buffer, assuming it's
    //long enough
    if(length >= 3) {
        data = gpio_get_data();
        string[1] |= (data & 0xE0) << 7;
        string[2] |= (data & 0x1C) << 10;
        string[3] |= (data & 0x03) << 13;
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

//Handle commands recieved from master
void SPI2_IRQHandler(void) {
    pool_item_t received = SPI_I2S_ReceiveData(SPI2);

    if ((received & 0x8000) > 0) {
        switch ((received & 0x6000) >> 13) {
        case 0:
            //Input/Output Mask
            gpio_set_io_mask(received & 0x00FF);
            break;
        case 1:
            //Configuration
            gpio_set_conf(received & 0x00FF);
            break;
        case 2:
            //Data
            gpio_set_data(received & 0x00FF);
            break;
        default:
            //NOOP
            break;
        }
    }

    SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_IT_RXNE);
}
