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


/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "stdio.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Remove after re-implementation of uart_Sendstring */
#include "UartRingbuffer.h"

#include "bsp.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;


#define wifi_uart &huart1
#define uart_command &huart2

void *ledRed;
void *ledBlue;
void *ledOrange;
void *ledGreen;

SemaphoreHandle_t xMutex;

void vSendDataThingSpeakTask(void *pvParameters)
{

  uint8_t buffer[2];
  DHT_DataTypedef *tmp;

  volatile UBaseType_t uxHighWaterMark;


  for (;;)
  {
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );


    //xSemaphoreTake(xMutex, portMAX_DELAY);
    tmp = (DHT_DataTypedef *)pvParameters;

    buffer[0] = tmp->Temperature;
    buffer[1] = tmp->Humidity;

    ESP_Send_Multi("U6123BFR6YNW5I4V", 2, buffer);

    //xSemaphoreGive(xMutex);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    if (uxHighWaterMark < 150 ) Error_Handler();

    /* wait 15s between sends */
    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}

void vRefreshWebserverTask(void *pvParameters)
{
	ControlTempParams *control;

	volatile UBaseType_t uxHighWaterMark;



  for (;;)
  {
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );


    control = (ControlTempParams *)pvParameters;
    //xSemaphoreTake(xMutex, portMAX_DELAY);

//    Uart_sendstring("Empezando a refrescar la pagina\r\n", uart_command);
    RefreshWebserver(control);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    if (uxHighWaterMark < 150 ) Error_Handler();

    vTaskDelay(pdMS_TO_TICKS(500));

    //xSemaphoreGive(xMutex);
  }
}


void vControlTempHum(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  ControlTempParams *param;

  volatile UBaseType_t uxHighWaterMark;


  for (;;)
  {
	  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );


    // xSemaphoreTake( xMutex, portMAX_DELAY );
    param = (ControlTempParams *)pvParameters;

    // Temperature Control

    // Threshold activated
    if (param->temp_Struct.thresholdSet)
    {

      // temperature is lower than threshold
      if (param->dhtPolledData.Temperature < param->temp_Struct.min)
      {
        vSetTemp(param->temp_Struct.max);

        // temperature is higher than threshold
      }
      else if (param->dhtPolledData.Temperature > param->temp_Struct.max)
      {
        vSetTemp(param->temp_Struct.min);
      }

      // temperature value set
    }
    else if (param->temp_Struct.valueSet)
    {
      if (param->dhtPolledData.Temperature != param->temp_Struct.value)
      {
        vSetTemp(param->temp_Struct.value);
      }
    }

    // Humidity Control

    // Threshold activated
    if (param->hum_Struct.thresholdSet)
    {

      // temperature is lower than threshold
      if (param->dhtPolledData.Humidity < param->hum_Struct.min)
      {
        vSetHum(param->hum_Struct.max);

        // temperature is higher than threshold
      }
      else if (param->dhtPolledData.Humidity > param->hum_Struct.max)
      {
        vSetHum(param->hum_Struct.min);
      }

      // temperature value set
    }
    else if (param->hum_Struct.valueSet)
    {
      if (param->dhtPolledData.Humidity != param->hum_Struct.value)
      {
        vSetHum(param->hum_Struct.value);
      }
    }
    // xSemaphoreGive( xMutex);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    if (uxHighWaterMark < 150 ) Error_Handler();


    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(7000));
  }
}


void vTaskGetDataDHT(void *pvParameters)
{

  char buf[30];
  TickType_t xLastWakeTime = xTaskGetTickCount();
  DHT_DataTypedef *data_struct;

  volatile UBaseType_t uxHighWaterMark;


  for (;;)
  {
    //xSemaphoreTake(xMutex, portMAX_DELAY);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

    data_struct = (DHT_DataTypedef *)pvParameters;
    readDHTSensor(data_struct);

    sprintf(buf, "Temp: %u, Hum:%u\r\n", data_struct->Temperature, data_struct->Humidity);

    /* Improve this somehow */
    Uart_sendstring(buf, uart_command);

    //xSemaphoreGive(xMutex);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    if (uxHighWaterMark < 150 ) Error_Handler();

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));

  }
}

int main(void)
{
  BSP_Init();



  vConnectWifi_StaticIp("Diagon Alley 2.4GHz", "hayunboggartenlaalacena", "192.168.0.200");


  /* Initialize some params */

  static ControlTempParams param;

  param.dhtPolledData.Humidity = 10;
  param.dhtPolledData.Temperature = 10;

  param.temp_Struct.min = 20;
  param.temp_Struct.max = 40;
  param.temp_Struct.thresholdSet = true;
  param.temp_Struct.valueSet = false;

  param.hum_Struct.min = 90;
  param.hum_Struct.max = 95;
  param.temp_Struct.thresholdSet = true;
  param.hum_Struct.valueSet = false;

  int res1, res2, res3, res4;
  res1 = res2 = res3 = res4 = 0;

    xMutex = xSemaphoreCreateMutex();

  if (xMutex != NULL)
  {

    res1 = xTaskCreate(vTaskGetDataDHT, "vTaskGetData", 300, &param.dhtPolledData, 4, NULL);
    res2 = xTaskCreate(vControlTempHum, "controlTemp", 300, &param, 2, NULL);
    res3 = xTaskCreate(vRefreshWebserverTask, "RefreshWebserver", 1450, &param, 1, NULL);
    res4 = xTaskCreate(vSendDataThingSpeakTask, "SendDataThingSpeak", 500, &param.dhtPolledData, 3, NULL);
  }

  /* Check all task were created correctly */
  if (!((res1 == pdPASS) && (res2 == pdPASS) && (res3 == pdPASS) && (res4 == pdPASS)))
  {
    Error_Handler();
  }

  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  for (;;);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */


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
