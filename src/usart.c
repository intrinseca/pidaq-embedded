#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "usart.h"

void usart_init() {
	USART_InitTypeDef usart_params;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_StructInit(&usart_params);
	usart_params.USART_BaudRate = 115200;

	USART_Init(USART1, &usart_params);

    USART_Cmd(USART1, ENABLE);
}

void usart_send(char* string) {
    do {
        USART_SendData(USART1, *string);
        string++;
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
    } while (*string);
}
