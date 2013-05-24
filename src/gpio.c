#include "stm32f10x_conf.h"
#include "gpio.h"

void _gpio_configure();

char curr_io_mask;
char curr_configuration;

void gpio_init(){
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
