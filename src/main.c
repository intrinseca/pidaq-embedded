#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "usart.h"
#include "spi.h"
#include "pool.h"
#include "adc.h"
#include "gpio.h"

int main(void) {
    pool_item_t * current_buf = 0;
    pool_item_t * new_buf = 0;

    //Make the sample timer stop when paused for debugging
    DBGMCU_Config(DBGMCU_TIM2_STOP, ENABLE);

    //Configure Interrupt Priority
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    //Make sure the system clock value is set correctly
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    SystemCoreClock = RCC_ClocksStatus.SYSCLK_Frequency;

    //Turn on DMA1 (used by several modules)
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    //Initialise GPIO
    gpio_init();

    //Setup SysTick interrupt at 1ms intervals
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1)
            ;
    }

    //Initialise sample buffers
    pool_init();

    //Enable Communications
    usart_init();
    spi_init();

    //Initialise sampling
    adc_init();

    usart_send("\n\nPiDAQ r1\n");

    //Start sampling
    adc_start();

    char num[] = "0d\n";

    while (1) {
        GPIO_WriteBit(GPIOB, GPIO_Pin_1, (alloced_num > 4));

        if (spi_tx_done) {
            if (current_buf) {
                adc_free_buff(current_buf);

                num[0] = '0' + alloced_num;
                usart_send(num);
                current_buf = 0;
            }

            new_buf = adc_get_filled_buff();

            if (new_buf) {
                usart_send("c");
                current_buf = new_buf;

                if (spi_send_string(current_buf, POOL_BUFF_SIZE)) {
                    usart_send("f");
                }
                else {
                    usart_send("!");
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
