#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "init.h"
#include "spi.h"
#include "pool.h"
#include "samples.h"

void send_usart(char* string) {
	do {
		USART_SendData(USART1, *string);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE ) == RESET)
			;
	} while (*string++);
}

int main(void) {
	char * buf = 0;

	//Initialise Sample Buffers
	pool_init();
	adc_init();

	//Initialise Peripherals
	init_rcc();
	init_gpio();
	init_usart();
	init_spi();
	init_timer();

	//Enable USART
	USART_Cmd(USART1, ENABLE);

	send_spi("\n\nPiDAQ r1\n", 11);
	send_usart("\n\nPiDAQ r1\n");

	//TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	while (1) {
		if (!spi_busy()) {
			if (buf)
				adc_free_buff(buf);

			buf = adc_get_filled_buff();

			if (buf) {
				send_spi(buf, POOL_BUFF_SIZE);
			}
		}
	}
}

void TIM2_IRQHandler(void) {
	static uint8_t state = 0;

	if (state == 0) {
		GPIOB ->BSRR = 1 << 1;
		send_usart("A");
		send_spi("On", 2);
		state = 1;
	} else {
		GPIOB ->BRR = 1 << 1;
		send_usart("B\n");
		send_spi("Off", 3);
		state = 0;
	}

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update );
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
