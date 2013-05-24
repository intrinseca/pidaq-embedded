#ifndef GPIO_H_
#define GPIO_H_

#define PIN_INPUT 0
#define PIN_OUTPUT 1

#define PIN_INPUT_PD 0
#define PIN_INPUT_PU 1

#define PIN_OUTPUT_PP 0
#define PIN_OUTPUT_OD 1

void gpio_init();

void gpio_set_io_mask(char mask);
void gpio_set_conf(char configuration);
void gpio_set_data(char data);

char gpio_get_data();

#endif
