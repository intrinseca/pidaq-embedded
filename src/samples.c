#include "stm32f10x.h"
#include "stm32f10x_tim.h"

#include "pool.h"
#include "samples.h"

uint8_t get_new_buff(void);

unsigned char * curr_buff;
volatile unsigned char next_free_pos;

volatile unsigned char * filled_buff;

void adc_init(void) {
	get_new_buff();
}

void TIM3_IRQHandler(void) {
	uint8_t sample;

	/* check if current buffer has space */
	if (next_free_pos < POOL_BUFF_SIZE) {
		/* no buffer management needed */
	} else {
		/* buffer is full. If the previous buffer has already been 'claimed'
		 * some other system (e.g. USB), allocate a new buffer and mark the
		 * current one as full. If the previous buffer has not been claimed,
		 * then just refill the current buffer. If the old one has been
		 * claimed, but we cannot allocate a new buffer, reuse current
		 * buffer, and mark overrun.
		 */
		if (filled_buff == 0) { // a.k.a has been claimed
			unsigned char * new_buff;
			new_buff = pool_malloc_buff();

			if (new_buff != 0) { // successfully got a new buffer
				filled_buff = (volatile void *) curr_buff;
				curr_buff = new_buff;
				next_free_pos = 0;
			} else { // no free buffers available, reuse existing buffer
				next_free_pos = 0;  // this represents an overrun
				// FIXME: count overrun
			}
		} else { // previous buffer hasn't been claimed, just rewrite
			next_free_pos = 0;
		}
	}

	//generate a sample
	sample = '0' + next_free_pos;

	// copy sample into buffer
	curr_buff[next_free_pos] = sample;
	next_free_pos++;

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update );
}

void * adc_get_filled_buff(void)
{
    void * ret;

    //TODO: Disable Interrupts
	ret = (void *) filled_buff;
	filled_buff = 0;

    return ret;
}

void adc_free_buff(void * buff)
{
    pool_free_buff(buff);
}

/*
 * Gets a new buffer from the pool and sets internal state. Returns TRUE on
 * success, otherwise FALSE
 */
uint8_t get_new_buff(void)
{
    void * new_buff;

    new_buff = pool_malloc_buff();

    if(new_buff == 0) {
        return 0;  // no buffers available
    }

    curr_buff = new_buff;
    next_free_pos = 0;

    return 1;
}
