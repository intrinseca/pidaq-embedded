#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "init.h"
#include "spi.h"
#include "pool.h"
#include "adc.h"

int main(void) {
    char * current_buf = 0;
    char * new_buf = 0;

    //Make the sample timer stop when paused for debugging
    DBGMCU_Config(DBGMCU_TIM2_STOP, ENABLE);

    //Configure Interrupt Priority
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    //Make sure the system clock value is set correctly
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    SystemCoreClock = RCC_ClocksStatus.SYSCLK_Frequency;

    //Initialise Core/Common Peripherals
    init_rcc();
    init_gpio();

    //Setup SysTick interrupt at 1ms intervals
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1)
            ;
    }

    //Initialise sample buffers
    pool_init();

    //Enable Communications
    init_usart();
    USART_Cmd(USART1, ENABLE);
    spi_init();

    //Initialise sampling
    adc_init();

    send_usart("\n\nPiDAQ r1\n");

    //Start sampling
    adc_start();

    while (1) {
        if (spi_tx_done) {
            if (current_buf) {
                adc_free_buff(current_buf);
                send_usart("d\n");
                current_buf = 0;
            }

            new_buf = adc_get_filled_buff();

            if (new_buf) {
                send_usart("c");
                current_buf = new_buf;

                if (spi_send_string(current_buf, POOL_BUFF_SIZE)) {
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
