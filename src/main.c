#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "init.h"

#define START_SPEED 21

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

	//Initialise Peripherals
	init_rcc();
	init_gpio();
	init_usart();
	init_spi();

	//Enable SPI and USART
	SPI_Cmd(SPI2, ENABLE);
	USART_Cmd(USART1, ENABLE);

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
