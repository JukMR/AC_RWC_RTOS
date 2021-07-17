/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"
#include "stdbool.h"
#include "FreeRTOS.h"
//#include "Task.h"



#include "led.h"
#include "ESP_DATA_HANDLER.h"
#include "ESPDataLogger.h"
#include "DHT.h"
#include "stdio.h"

// Remove after re-implementation of uart_Sendstring
#include "UartRingbuffer.h"

#define uart_command huart2

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);




DHT_DataTypedef DHT11_Data;
float temp;
float hum;

typedef struct
{
  bool valueSet;
  int value;
  bool thresholdSet;
  int min;
  int max;
} Threshold_TypeDef;

typedef struct
{
  DHT_DataTypedef dhtPolledData;
  Threshold_TypeDef temp_Struct;
  Threshold_TypeDef hum_Struct;
} ControlTempParams;

void pull_dht_data(float *temp, float *hum)
{
  DHT_GetData(&DHT11_Data);
  *temp = DHT11_Data.Temperature;
  *hum = DHT11_Data.Humidity;
}

float buffer[2];

// Agregar un buffer sin condiciones de carrera para esto
void vPollDHT(float *buffer)
{

//  LED_blinky(ledGreen, 1, 1, 2);

  pull_dht_data(&temp, &hum);

  buffer[0] = temp;
  buffer[1] = hum;
}

void vSendDataThingSpeak(float *buffer)
{

  TickType_t xLastWakeTime = xTaskGetTickCount();

  ESP_Send_Multi("U6123BFR6YNW5I4V", 2, buffer);

  // wait 15s between sends
  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 15000 ));

}

void vRefreshWebserver(float *buffer)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  Server_Start();

  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 2500 ));
}

void vSetTemp(int Value)
{
  char buffer[40] = {0};
  sprintf(buffer, "do=setTemp,%d.\r\n", Value);

  // Implement this
  Uart_sendstring(buffer, &uart_command);
}

void vSetHum(int Value)
{
  char buffer[40] = {0};
  sprintf(buffer, "do=setHum,%d.\r\n", Value);

  // Implement this
  Uart_sendstring(buffer, &uart_command);
}

void vControlTempHum(ControlTempParams *param1)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();


  // Temperature Control

  // Threshold activated
  if (param1->temp_Struct.thresholdSet)
  {

    // temperature is lower than threshold
    if (param1->dhtPolledData.Temperature < param1->temp_Struct.min)
    {
      vSetTemp(param1->temp_Struct.max);

      // temperature is higher than threshold
    }
    else if (param1->dhtPolledData.Temperature > param1->temp_Struct.max)
    {
      vSetTemp(param1->temp_Struct.min);
    }

    // temperature value set
  }
  else if (param1->temp_Struct.valueSet)
  {
    if (param1->dhtPolledData.Temperature != param1->temp_Struct.value)
    {
      vSetTemp(param1->temp_Struct.value);
    }
  }

  // Humidity Control

  // Threshold activated
  if (param1->hum_Struct.thresholdSet)
  {

    // temperature is lower than threshold
    if (param1->dhtPolledData.Humidity < param1->hum_Struct.min)
    {
      vSetHum(param1->hum_Struct.max);

      // temperature is higher than threshold
    }
    else if (param1->dhtPolledData.Humidity > param1->hum_Struct.max)
    {
      vSetHum(param1->hum_Struct.min);
    }

    // temperature value set
  }
  else if (param1->hum_Struct.valueSet)
  {
    if (param1->dhtPolledData.Humidity != param1->hum_Struct.value)
    {
      vSetHum(param1->hum_Struct.value);
    }
  }

  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 3000 ));
}

void vTurnOn(){
  char buffer[20] = {0};
  sprintf(buffer, "do=turnon.\r\n");
  Uart_sendstring(buffer, &uart_command);

}

void vTurnOff(){
  char buffer[20] = {0};
  sprintf(buffer, "do=turnoff.\r\n");
  Uart_sendstring(buffer, &uart_command);

}

ControlTempParams *param1;

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();

  MX_FREERTOS_Init();

  Threshold_TypeDef temp;
  Threshold_TypeDef hum;


  xTaskCreate( vControlTempHum, "controlTemp", 1000, param1, 2, NULL);
  xTaskCreate( vSendDataThingSpeak, "SendDataThingSpeak", 1000, &buffer, 1, NULL);
  xTaskCreate( vRefreshWebserver, "RefreshWebserver", 1000, &buffer, 1, NULL);
  vTaskStartScheduler();

  while (1);
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
