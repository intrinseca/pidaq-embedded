#include "stm32f10x.h"
#include "stm32f10x_tim.h"

#include "pool.h"
#include "samples.h"
#include "spi.h"

uint8_t get_new_buff(void);

unsigned char * curr_buff;
volatile unsigned char next_free_pos;

volatile unsigned char * filled_buffs[POOL_NUM_BUFFERS];
unsigned char filled_buff_head;
unsigned char filled_buff_tail;

void adc_init(void) {
	int i;

	get_new_buff();
	filled_buff_head = 0;
	filled_buff_tail = 0;

	for (i = 0; i < POOL_NUM_BUFFERS; i++) {
		filled_buffs[i] = 0;
	}
}

void TIM3_IRQHandler(void) {
	uint8_t sample;
	static uint8_t count = 0;
	char status_message[] = "X0";

	/* check if current buffer has space */
	if (next_free_pos < POOL_BUFF_SIZE) {
		/* no buffer management needed */
	}
	else {
		/* buffer is full. If the previous buffer has already been 'claimed'
		 * some other system (e.g. USB), allocate a new buffer and mark the
		 * current one as full. If the previous buffer has not been claimed,
		 * then just refill the current buffer. If the old one has been
		 * claimed, but we cannot allocate a new buffer, reuse current
		 * buffer, and mark overrun.
		 */
		if (filled_buffs[filled_buff_head] == 0) { // a.k.a has been claimed
			unsigned char * new_buff;
			new_buff = pool_malloc_buff();

			if (new_buff != 0) { // successfully got a new buffer
				filled_buffs[filled_buff_head] = (volatile void *) curr_buff;
				filled_buff_head++;
				if (filled_buff_head >= POOL_NUM_BUFFERS)
					filled_buff_head = 0;

				curr_buff = new_buff;
				next_free_pos = 0;
				status_message[1] = '0' + alloced_num;
				send_usart(status_message);
			}
			else { // no free buffers available, reuse existing buffer
				next_free_pos = 0;  // this represents an overrun
				send_usart("P");
				// FIXME: count overrun
			}
		}
		else { // previous buffer hasn't been claimed, just rewrite
			send_usart("U");
			next_free_pos = 0;
		}
	}

	//generate a sample
	sample = count;
	count++;

	// copy sample into buffer
	curr_buff[next_free_pos] = sample;
	next_free_pos++;

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update );
}

void * adc_get_filled_buff(void) {
	void * ret;

	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
	ret = (void *) filled_buffs[filled_buff_tail];
	filled_buffs[filled_buff_tail] = 0;
	filled_buff_tail++;
	if (filled_buff_tail >= POOL_NUM_BUFFERS)
		filled_buff_tail = 0;

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	return ret;
}

void adc_free_buff(void * buff) {
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
	pool_free_buff(buff);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

/*
 * Gets a new buffer from the pool and sets internal state. Returns TRUE on
 * success, otherwise FALSE
 */
uint8_t get_new_buff(void) {
	void * new_buff;

	new_buff = pool_malloc_buff();

	if (new_buff == 0) {
		return 0;  // no buffers available
	}

	curr_buff = new_buff;
	next_free_pos = 0;

	return 1;
}
