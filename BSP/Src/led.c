#include "led.h"

GPIOLED_TypeDef gpio_ledRed = {GPIOD, GPIO_PIN_14};
GPIOLED_TypeDef gpio_ledBlue = {GPIOD, GPIO_PIN_15};
GPIOLED_TypeDef gpio_ledOrange = {GPIOD, GPIO_PIN_13};
GPIOLED_TypeDef gpio_ledGreen = {GPIOD, GPIO_PIN_12};

void vLED_on(void *led){

	GPIOLED_TypeDef *gpioLed;

	gpioLed = (GPIOLED_TypeDef*)led;
	HAL_GPIO_WritePin(gpioLed->port, gpioLed->pin, GPIO_PIN_SET);
}

void vLED_off(void *led){

	GPIOLED_TypeDef *gpioLed;

	gpioLed = (GPIOLED_TypeDef*)led;
	HAL_GPIO_WritePin(gpioLed->port, gpioLed->pin, GPIO_PIN_RESET);
}


void vLED_Init(void){

	GPIO_InitTypeDef  GPIO_InitStruct;

	__HAL_RCC_GPIOD_CLK_ENABLE();

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	GPIO_InitStruct.Pin = gpio_ledRed.pin;
	HAL_GPIO_Init(gpio_ledRed.port, &GPIO_InitStruct);
	vLedRed = (void*)&gpio_ledRed;

	GPIO_InitStruct.Pin = gpio_ledBlue.pin;
	HAL_GPIO_Init(gpio_ledBlue.port, &GPIO_InitStruct);
	vLedBlue = (void*)&gpio_ledBlue;

	GPIO_InitStruct.Pin = gpio_ledOrange.pin;
	HAL_GPIO_Init(gpio_ledOrange.port, &GPIO_InitStruct);
	vLedOrange = (void*)&gpio_ledOrange;

	GPIO_InitStruct.Pin = gpio_ledGreen.pin;
	HAL_GPIO_Init(gpio_ledGreen.port, &GPIO_InitStruct);
	vLedGreen = (void*)&gpio_ledGreen;
}


void vLED_toggle(void *led){
	GPIOLED_TypeDef *gpioLed;

	gpioLed = (GPIOLED_TypeDef*)led;
	HAL_GPIO_TogglePin(gpioLed->port, gpioLed->pin);
}
