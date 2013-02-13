#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"

#define START_SPEED 21

GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
SPI_InitTypeDef SPI_InitStructure;

void send_usart(char* string) {
	do {
		USART_SendData(USART1, *string);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE ) == RESET)
			;
	} while (*string++);
}

void spi_send_frame(uint16_t data) {
	SPI_I2S_SendData(SPI2, data);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE ) == RESET)
		;
}

void send_spi(char* string, uint16_t length) {
	spi_send_frame(length);
	while (length > 0) {
		spi_send_frame(*string);
		string++;
		length--;
	}

	SPI_I2S_SendData(SPI2, 0);
}

int main(void) {
	int i;
	int j;
	/*!< At this stage the microcontroller clock setting is already configured,
	 this is done through SystemInit() function which is called from startup
	 file (startup_stm32f10x_xx.s) before to branch to application main.
	 To reconfigure the default setting of SystemInit() function, refer to
	 system_stm32f10x.c file
	 */

	/* GPIOD Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Configure PD0 and PD2 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);

	SPI_StructInit(&SPI_InitStructure);
	SPI_Init(SPI2, &SPI_InitStructure);

	SPI_Cmd(SPI2, ENABLE);
	USART_Cmd(USART1, ENABLE);

	/* To achieve GPIO toggling maximum frequency, the following  sequence is mandatory.
	 You can monitor PD0 or PD2 on the scope to measure the output signal.
	 If you need to fine tune this frequency, you can add more GPIO set/reset
	 cycles to minimize more the infinite loop timing.
	 This code needs to be compiled with high speed optimization option.  */

	j = 1 << START_SPEED;
	send_spi("\n\nPiDAQ r1\n", 11);
	send_usart("\n\nPiDAQ r1\n");

	while (1) {
		GPIOB ->BSRR = 1 << 0;
		GPIOB ->BRR = 1 << 1;
		send_usart("A");
		send_spi("678901678901", 12);

		i = j;
		while (--i)
			;

		// Reset PC12
		GPIOB ->BRR = 1 << 0;
		GPIOB ->BSRR = 1 << 1;
		send_usart("B\n");
		send_spi("234567234567", 12);

		i = j;
		while (--i)
			;
		//j = j >> 1;

		if (j == 1)
			j = 1 << START_SPEED;
	}
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}

#endif
