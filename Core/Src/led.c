#include "led.h"


void LED_on(void *led){

	GPIOLED_TypeDef *gpioLed;

	gpioLed = (GPIOLED_TypeDef*)led;
	HAL_GPIO_WritePin(gpioLed->port, gpioLed->pin, GPIO_PIN_SET);
}

void LED_off(void *led){

	GPIOLED_TypeDef *gpioLed;

	gpioLed = (GPIOLED_TypeDef*)led;
	HAL_GPIO_WritePin(gpioLed->port, gpioLed->pin, GPIO_PIN_RESET);
}


void LED_Init(void){

	GPIO_InitTypeDef  GPIO_InitStruct;

	__HAL_RCC_GPIOD_CLK_ENABLE();

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	GPIO_InitStruct.Pin = gpio_ledRed.pin;
	HAL_GPIO_Init(gpio_ledRed.port, &GPIO_InitStruct);
	ledRed = (void*)&gpio_ledRed;

	GPIO_InitStruct.Pin = gpio_ledBlue.pin;
	HAL_GPIO_Init(gpio_ledBlue.port, &GPIO_InitStruct);
	ledBlue = (void*)&gpio_ledBlue;

	GPIO_InitStruct.Pin = gpio_ledOrange.pin;
	HAL_GPIO_Init(gpio_ledOrange.port, &GPIO_InitStruct);
	ledOrange = (void*)&gpio_ledOrange;

	GPIO_InitStruct.Pin = gpio_ledGreen.pin;
	HAL_GPIO_Init(gpio_ledGreen.port, &GPIO_InitStruct);
	ledGreen = (void*)&gpio_ledGreen;
}


void LED_toggle(void *led){
	GPIOLED_TypeDef *gpioLed;

	gpioLed = (GPIOLED_TypeDef*)led;
	HAL_GPIO_TogglePin(gpioLed->port, gpioLed->pin);
}

//void LED_blinky(void *led, uint16_t ton, uint16_t toff, uint16_t times){
//
//	ledBlinky = led;
//	timesBlinky = times;
//	tonBlinky = ton;
//	toffBlinky = toff;
//}
//
//void LED_blinkyIRQ(void){
//	static uint16_t ton = 1;
//	static uint16_t toff = 1;
//
//	if(timesBlinky){
//		if(ton){
//			ton--;
//			if(!ton){
//				LED_off(ledBlinky);
//			}
//		} else if(toff){
//			toff--;
//			if(!toff){
//				timesBlinky--;
//				if(timesBlinky){
//					ton = tonBlinky;
//					toff = toffBlinky;
//					LED_on(ledBlinky);
//				}
//			}
//		}
//	}
//}
