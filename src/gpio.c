#include "stm32f10x_conf.h"
#include "gpio.h"

void _gpio_configure();

char curr_io_mask;
char curr_configuration;

void gpio_init(){
    //Turn on the clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    //Set up other GPIO pins
    GPIO_InitTypeDef gpio_params;

    GPIO_StructInit(&gpio_params);
    gpio_params.GPIO_Speed = GPIO_Speed_50MHz;

    //PB0, PB1 Outputs
    gpio_params.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio_params.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpio_params);

    //PA9 USART1 Tx
    gpio_params.GPIO_Pin = GPIO_Pin_9;
    gpio_params.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_params);

    //PA10 USART1 Rx
    gpio_params.GPIO_Pin = GPIO_Pin_10;
    gpio_params.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_params);

    //PB13 SPI2 SCK, PB15 SPI2 MOSI
    gpio_params.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    gpio_params.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio_params);

    //PB14 SPI2 MISO
    gpio_params.GPIO_Pin = GPIO_Pin_14;
    gpio_params.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpio_params);

    //PA0-3 AIN
    gpio_params.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4;
    gpio_params.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio_params);

    //Set up digital I/O pins
    curr_io_mask = 0;
    curr_configuration = 0;
    _gpio_configure();
}

void gpio_set_io_mask(char mask) {
    curr_io_mask = mask;
}

void gpio_set_conf(char configuration) {
    curr_configuration = configuration;
    _gpio_configure();
}

void gpio_set_data(char data)
{
    GPIO_Write(GPIOC, data);
}

char gpio_get_data()
{
    return (char) GPIO_ReadInputData(GPIOC);
}

void _gpio_configure()
{
    int i, pin, pin_mask, pin_conf;

    GPIO_InitTypeDef gpio_params;
    GPIO_StructInit(&gpio_params);
    gpio_params.GPIO_Speed = GPIO_Speed_50MHz;

    for (i = 0, pin = 1; i < 8; i++, pin = pin << 1) {
        pin_mask = (curr_io_mask & pin) >> i;
        pin_conf = (curr_configuration & pin) >> i;

        gpio_params.GPIO_Pin = pin;

        if(pin_mask == PIN_INPUT)
        {
            if(pin_conf == PIN_INPUT_PD)
                gpio_params.GPIO_Mode = GPIO_Mode_IPD;
            else if(pin_conf == PIN_INPUT_PU)
                gpio_params.GPIO_Mode = GPIO_Mode_IPU;
        }
        else
        {
            if(pin_conf == PIN_OUTPUT_PP)
                gpio_params.GPIO_Mode = GPIO_Mode_Out_PP;
            else if(pin_conf == PIN_OUTPUT_OD)
                gpio_params.GPIO_Mode = GPIO_Mode_Out_OD;
        }

        GPIO_Init(GPIOC, &gpio_params);
    }
}
