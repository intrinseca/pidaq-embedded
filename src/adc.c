#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "pool.h"
#include "adc.h"
#include "spi.h"
#include "usart.h"

volatile pool_item_t * filled_buffs[POOL_NUM_BUFFERS];
unsigned char filled_buff_head;
unsigned char filled_buff_tail;

#define DMA_BUFF_LENGTH (POOL_BUFF_SIZE - 1)

pool_item_t dma_buff[2 * DMA_BUFF_LENGTH] = { 0x00, };

void adc_init_timer() {
    TIM_TimeBaseInitTypeDef tim_params;
    TIM_OCInitTypeDef tim_oc_params;
    NVIC_InitTypeDef nvic_params;
    DMA_InitTypeDef dma_params;
    ADC_InitTypeDef adc_params;

    //Set up clocks
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    //Set up TIM2 to trigger ADC
    //APB1 runs at half the system clock, but the feed to TIM2/4 is doubled
    //Prescaler sets period to 10us
    //Period is then 50us / 20 kHz
    TIM_TimeBaseStructInit(&tim_params);
    tim_params.TIM_Prescaler = SystemCoreClock / 100000;
    tim_params.TIM_Period = 5;
    TIM_TimeBaseInit(TIM2, &tim_params);

    //Set up the capture/compare to provide the ADC trigger
    TIM_OC2Init(TIM2, &tim_oc_params);
    tim_oc_params.TIM_OCMode = TIM_OCMode_PWM1;
    tim_oc_params.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_params.TIM_Pulse = 2;
    tim_oc_params.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC2Init(TIM2, &tim_oc_params);

    //Configure the DMA
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA_StructInit(&dma_params);
    dma_params.DMA_PeripheralBaseAddr = (uint32_t) (&(ADC1->DR));
    dma_params.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma_params.DMA_Priority = DMA_Priority_High;
    dma_params.DMA_MemoryBaseAddr = (uint32_t) dma_buff;
    dma_params.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_params.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_params.DMA_BufferSize = 2 * DMA_BUFF_LENGTH;
    dma_params.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_params.DMA_Mode = DMA_Mode_Circular;
    DMA_Init(DMA1_Channel1, &dma_params);

    //Enable DMA complete interrupt
    nvic_params.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    nvic_params.NVIC_IRQChannelPreemptionPriority = 3;
    nvic_params.NVIC_IRQChannelSubPriority = 0;
    nvic_params.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_params);

    DMA_ClearITPendingBit(DMA1_IT_TC1 );
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT, ENABLE);

    //Enable the DMA controller
    DMA_Cmd(DMA1_Channel1, ENABLE);

    //ADC1 configuration
    adc_params.ADC_Mode = ADC_Mode_Independent;
    adc_params.ADC_ScanConvMode = DISABLE;
    adc_params.ADC_ContinuousConvMode = DISABLE;
    adc_params.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;
    adc_params.ADC_DataAlign = ADC_DataAlign_Right;
    adc_params.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc_params);

    //ADC1 channel configuration
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    //Enable ADC1 DMA
    ADC_DMACmd(ADC1, ENABLE);

    //Enable ADC1 external trigger
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    //Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void) {
    pool_item_t * new_buff;
    pool_item_t * dma_buff_start;
    int i = 0;

    //Work out if this is half or fully-complete
    //Clear appropriate flag and get buffer start location
    if (DMA_GetITStatus(DMA1_IT_HT1 ) == SET) {
        dma_buff_start = dma_buff;
        DMA_ClearITPendingBit(DMA1_IT_HT1 );
        usart_send("<");
    }
    else {
        dma_buff_start = &dma_buff[DMA_BUFF_LENGTH];
        DMA_ClearITPendingBit(DMA1_IT_TC1 );
        usart_send(">");
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

        usart_send("X");
    }
    else {
        //Overflow or out of memory, in either case, nowhere to put the data
        usart_send("0");
    }
}

void adc_start() {
    TIM_Cmd(TIM2, ENABLE);
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
