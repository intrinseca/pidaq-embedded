#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "init.h"

void init_rcc() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

void init_gpio() {
	GPIO_InitTypeDef gpio_params;

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
}

void init_usart() {
	USART_InitTypeDef usart_params;

	USART_StructInit(&usart_params);
	usart_params.USART_BaudRate = 115200;

	USART_Init(USART1, &usart_params);
}