#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"

#include "init.h"
#include "spi.h"
#include "pool.h"
#include "samples.h"

int main(void) {
	char * current_buf = 0;
	char * new_buf = 0;

	//Configure Interrupt Priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	SystemCoreClock = RCC_ClocksStatus.SYSCLK_Frequency;

	//Core Peripherals
	init_rcc();
	init_gpio();

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK );
	//Setup SysTick interrupt at 1ms intervals
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1)
			;
	}

	//Initialise Sample Buffers
	pool_init();

	//Enable USART
	init_usart();
	USART_Cmd(USART1, ENABLE);

	adc_init();

	//Initialise Peripherals
	spi_init();
	init_timer();

	send_usart("\n\nPiDAQ r1\n");

	//TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	while (1) {
		if (spi_tx_done) {
			if(current_buf)
			{
				send_usart("d\n");
				current_buf = 0;
			}

			new_buf = adc_get_filled_buff();

			if (new_buf) {
				send_usart("c");
				current_buf = new_buf;

				if (spi_send_string(current_buf, POOL_BUFF_SIZE)) {
					adc_free_buff(new_buf);
					send_usart("f");
				}
				else {
					send_usart("!");
				}
			}
		}
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
