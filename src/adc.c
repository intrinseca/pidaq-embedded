#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "pool.h"
#include "adc.h"
#include "spi.h"

volatile pool_item_t * filled_buffs[POOL_NUM_BUFFERS];
unsigned char filled_buff_head;
unsigned char filled_buff_tail;

#define DMA_BUFF_LENGTH (POOL_BUFF_SIZE - 1)

pool_item_t dma_buff[2 * DMA_BUFF_LENGTH] = { 0x00, };

void adc_init_timer() {
    TIM_TimeBaseInitTypeDef tim_params;
    NVIC_InitTypeDef nvic_params;
    DMA_InitTypeDef dma_params;

    //Set up TIM3 for 'sampling'
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    //APB1 runs at half the system clock, but the feed to TIM2/4 is doubled
    //Prescaler sets period to 10us
    //Period is then 50us / 20 kHz
    TIM_TimeBaseStructInit(&tim_params);
    tim_params.TIM_Prescaler = SystemCoreClock / 100000;
    tim_params.TIM_Period = 5;
    TIM_TimeBaseInit(TIM2, &tim_params);

    //TIM2 elapsed triggers a DMA request
    //TODO: Trigger ADC
    TIM_SelectCCDMA(TIM2, ENABLE);
    TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);

    //Temporary measure: Make TIM2 update TIM4 to use TIM4 as 'sample'
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update );

    TIM_TimeBaseStructInit(&tim_params);
    tim_params.TIM_Period = 1023;
    TIM_TimeBaseInit(TIM4, &tim_params);

    TIM_ITRxExternalClockConfig(TIM4, TIM_TS_ITR1 );

    //Configure the DMA
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_StructInit(&dma_params);
    dma_params.DMA_PeripheralBaseAddr = (uint32_t) (&(TIM4 ->CNT));
    dma_params.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma_params.DMA_Priority = DMA_Priority_High;
    dma_params.DMA_MemoryBaseAddr = (uint32_t) dma_buff;
    dma_params.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_params.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_params.DMA_BufferSize = 2 * DMA_BUFF_LENGTH;
    dma_params.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_params.DMA_Mode = DMA_Mode_Circular;
    DMA_Init(DMA1_Channel2, &dma_params);

    //Enable DMA complete interrupt
    nvic_params.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    nvic_params.NVIC_IRQChannelPreemptionPriority = 3;
    nvic_params.NVIC_IRQChannelSubPriority = 0;
    nvic_params.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_params);

    DMA_ClearITPendingBit(DMA1_IT_TC2 );
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel2, DMA_IT_HT, ENABLE);

    //Enable the DMA controller
    DMA_Cmd(DMA1_Channel2, ENABLE);
}

void DMA1_Channel2_IRQHandler(void) {
    pool_item_t * new_buff;
    pool_item_t * dma_buff_start;
    int i = 0;

    //Work out if this is half or fully-complete
    //Clear appropriate flag and get buffer start location
    if (DMA_GetITStatus(DMA1_IT_HT2 ) == SET) {
        dma_buff_start = dma_buff;
        DMA_ClearITPendingBit(DMA1_IT_HT2 );
        send_usart("<");
    }
    else {
        dma_buff_start = &dma_buff[DMA_BUFF_LENGTH];
        DMA_ClearITPendingBit(DMA1_IT_TC2 );
        send_usart(">");
    }

    //Get a new buffer
    new_buff = pool_malloc_buff();

    //Check for out-of-memory and overflow
    if(new_buff != 0 && filled_buffs[filled_buff_head] == 0)
    {
        //First value is length with high bit set to indicate header
        new_buff[0] = DMA_BUFF_LENGTH | 0x8000;

        //Copy the data in
        while(i < DMA_BUFF_LENGTH)
        {
            new_buff[i + 1] = dma_buff_start[i];
            i++;
        }

        //Add to filled buffer queue
        filled_buffs[filled_buff_head] = (volatile void *) new_buff;
        filled_buff_head++;
        if (filled_buff_head >= POOL_NUM_BUFFERS)
            filled_buff_head = 0;

        send_usart("X");
    }
    else {
        //Overflow or out of memory, in either case, nowhere to put the data
        send_usart("0");
    }
}

void adc_start() {
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void adc_init(void) {
    int i;

    //Initialise the filled buffer queue
    filled_buff_head = 0;
    filled_buff_tail = 0;

    for (i = 0; i < POOL_NUM_BUFFERS; i++) {
        filled_buffs[i] = 0;
    }

    adc_init_timer();
}

pool_item_t * adc_get_filled_buff(void) {
    void * ret;

    //FIXME: Only disable the DMA interrupt
    __disable_irq();

    ret = (void *) filled_buffs[filled_buff_tail];
    filled_buffs[filled_buff_tail] = 0;
    filled_buff_tail++;
    if (filled_buff_tail >= POOL_NUM_BUFFERS)
        filled_buff_tail = 0;

    __enable_irq();
    return ret;
}

void adc_free_buff(void * buff) {
    //Disable interrupts as malloc in the middle of free is a *bad* idea
    //FIXME: Only disable the DMA interrupt
    __disable_irq();

    pool_free_buff(buff);

    __enable_irq();
}
