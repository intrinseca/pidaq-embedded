/*
 * samples.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Michael
 */

#ifndef SAMPLES_H_
#define SAMPLES_H_

void adc_init(void);
void adc_start(void);
void * adc_get_filled_buff(void);
void adc_free_buff(void * buff);

#endif /* SAMPLES_H_ */
