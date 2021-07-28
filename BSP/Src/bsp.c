#include <esp8266.h>
#include <stdio.h>

#include "bsp.h"
#include "led.h"
#include "usart.h"
#include "gpio.h"
#include "UartRingbuffer.h"
#include "DHT.h"

#define wifi_uart &huart1
#define uart_command &huart2


void vSetTemp(uint8_t Value)
{
    char buffer[40] = {0};
    sprintf(buffer, "do=setTemp,%u.\r\n", Value);
    Uart_sendstring(buffer, uart_command);
}

void vSetHum(uint8_t Value)
{
    char buffer[40] = {0};
    sprintf(buffer, "do=setHum,%u.\r\n", Value);
    Uart_sendstring(buffer, uart_command);
}

void vTurnOn()
{
    char buffer[20] = {0};
    sprintf(buffer, "do=turnOn.\r\n");
    Uart_sendstring(buffer, uart_command);
}

void vTurnOff()
{
    char buffer[20] = {0};
    sprintf(buffer, "do=turnOff.\r\n");
    Uart_sendstring(buffer, uart_command);
}

void BSP_Init()
{

    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    vLED_Init();
}


void vConnectWifi_StaticIp(char *ssid, char *pass, char* ip){
    ESP_Init(ssid, pass, ip);
}

void vRefreshWebserver(xStateStructure_t *arg, xDelayTask_t *xSharedArgs){
	Server_Start(arg, xSharedArgs);
}

void vReadDHTSensor(DhtReadings_t *dataStruct){
    DHT_GetData(dataStruct);
}

void vLogDataThingSpeaker(char *api, int number_values, uint8_t *buffer){
	ESP_Send_Multi(api, number_values, buffer);
}

void vSendToUart(const char *str, UART_HandleTypeDef *uart){
	Uart_sendstring(str, uart);
}

void vTurnLedOn(void *led){
	vLED_on(led);
}

void vTurnLedOff(void *led){
	vLED_off(led);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
  */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 50;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
  */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}
