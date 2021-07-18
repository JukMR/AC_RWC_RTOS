#ifndef __led_H__
#define __led_H__

#include "stdint.h"
#include "stm32f4xx.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} GPIOLED_TypeDef;


extern void *ledRed;
extern void *ledBlue;
extern void *ledOrange;
extern void *ledGreen;

void LED_on(void *led);
void LED_off(void *led);
void LED_Init(void);


void LED_toggle(void *led);
//void LED_blinky(void *led, uint16_t ton, uint16_t toff, uint16_t times);
//void LED_blinkyIRQ(void);

#endif /* __led_H__ */
