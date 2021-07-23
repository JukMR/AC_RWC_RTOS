#ifndef __led_H__
#define __led_H__

#include "stdint.h"
#include "stm32f4xx.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} GPIOLED_TypeDef;


extern void *vLedRed;
extern void *vLedBlue;
extern void *vLedOrange;
extern void *vLedGreen;

void vLED_on(void *led);
void vLED_off(void *led);
void vLED_Init(void);


void vLED_toggle(void *led);

#endif /* __led_H__ */
