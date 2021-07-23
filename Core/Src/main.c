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
#include "timers.h"


#include "bsp.h"


#include "led.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;


#define wifi_uart &huart1
#define uart_command &huart2

void *vLedRed;
void *vLedBlue;
void *vLedOrange;
void *vLedGreen;

SemaphoreHandle_t xMutex;

typedef struct{
	  DHT_DataTypedef *pxDhtPolledData;
	  TimerHandle_t *pxTimer;
}task1_params;


void vSendDataThingSpeakTask(void *pvParameters)
{

  uint8_t uBuffer[2];
  DHT_DataTypedef *pxTmp;

  volatile UBaseType_t uxHighWaterMark;


  for (;;)
  {
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );


    //xSemaphoreTake(xMutex, portMAX_DELAY);
	pxTmp = (DHT_DataTypedef *)pvParameters;

    uBuffer[0] = pxTmp->uTemperature;
    uBuffer[1] = pxTmp->uHumidity;

    vLogDataThingSpeaker("U6123BFR6YNW5I4V", 2, uBuffer);

    //xSemaphoreGive(xMutex);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    if (uxHighWaterMark < 150 ) Error_Handler();

    /* wait 15s between sends */
    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}

void vRefreshWebserverTask(void *pvParameters)
{
	ControlTempParams *xControl;

	volatile UBaseType_t uxHighWaterMark;



  for (;;)
  {
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );


    xControl = (ControlTempParams *)pvParameters;
    //xSemaphoreTake(xMutex, portMAX_DELAY);

//    Uart_sendstring("Empezando a refrescar la pagina\r\n", uart_command);
    vRefreshWebserver(xControl);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    if (uxHighWaterMark < 150 ) Error_Handler();

    vTaskDelay(pdMS_TO_TICKS(500));

    //xSemaphoreGive(xMutex);
  }
}


void vControlTempHum(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  ControlTempParams *xParam;

  volatile UBaseType_t uxHighWaterMark;


  for (;;)
  {
	  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );


    // xSemaphoreTake( xMutex, portMAX_DELAY );
    xParam = (ControlTempParams *)pvParameters;

    // Temperature Control

    // Threshold activated
    if (xParam->xTemp_Struct.bThresholdSet)
    {

      // temperature is lower than threshold
      if (xParam->xDhtPolledData.uTemperature < xParam->xTemp_Struct.uMin)
      {
        vSetTemp(xParam->xTemp_Struct.uMax);

        // temperature is higher than threshold
      }
      else if (xParam->xDhtPolledData.uTemperature > xParam->xTemp_Struct.uMax)
      {
        vSetTemp(xParam->xTemp_Struct.uMin);
      }

      // temperature value set
    }
    else if (xParam->xTemp_Struct.bValueSet)
    {
      if (xParam->xDhtPolledData.uTemperature != xParam->xTemp_Struct.uValue)
      {
        vSetTemp(xParam->xTemp_Struct.uValue);
      }
    }

    // Humidity Control

    // Threshold activated
    if (xParam->xHum_Struct.bThresholdSet)
    {

      // temperature is lower than threshold
      if (xParam->xDhtPolledData.uHumidity < xParam->xHum_Struct.uMin)
      {
        vSetHum(xParam->xHum_Struct.uMax);

        // temperature is higher than threshold
      }
      else if (xParam->xDhtPolledData.uHumidity > xParam->xHum_Struct.uMax)
      {
        vSetHum(xParam->xHum_Struct.uMin);
      }

      // temperature value set
    }
    else if (xParam->xHum_Struct.bValueSet)
    {
      if (xParam->xDhtPolledData.uHumidity != xParam->xHum_Struct.uValue)
      {
        vSetHum(xParam->xHum_Struct.uValue);
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


  task1_params *xVar;
//  TimerHandle_t *timer;
  DHT_DataTypedef *xData_struct;

 volatile UBaseType_t uxHighWaterMark;

  for (;;)
  {
    //xSemaphoreTake(xMutex, portMAX_DELAY);
	vLED_on(vLedBlue);

   uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

   xVar = (task1_params *) pvParameters;
//    timer = var->timer;
    xData_struct = xVar->pxDhtPolledData;


    vReadDHTSensor(xData_struct);

    sprintf(buf, "Temp: %u, Hum:%u\r\n", xData_struct->uTemperature, xData_struct->uHumidity);

    /* Improve this somehow */
    vSendToUart(buf, uart_command);

    //xSemaphoreGive(xMutex);

   uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
   if (uxHighWaterMark < 150 ) Error_Handler();


    if (xTimerStart(*xVar->pxTimer, pdMS_TO_TICKS(1500) ) == pdFAIL){
    	/* Timer not initialized */
    	Error_Handler();
    }



    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));

  }
}



void vTimerTurnOn(){
	vLED_on(vLedBlue);

}

void vTimerTurnOff(){
	vLED_off(vLedBlue);

}


int main(void)
{
  BSP_Init();

  vConnectWifi_StaticIp("Diagon Alley 2.4GHz", "hayunboggartenlaalacena", "192.168.0.200");


  /* Initialize some params */

  static ControlTempParams xParam;

  xParam.xDhtPolledData.uHumidity = 10;
  xParam.xDhtPolledData.uTemperature = 10;

  xParam.xTemp_Struct.uMin = 20;
  xParam.xTemp_Struct.uMax = 40;
  xParam.xTemp_Struct.bThresholdSet = true;
  xParam.xTemp_Struct.bValueSet = false;

  xParam.xHum_Struct.uMin = 90;
  xParam.xHum_Struct.uMax = 95;
  xParam.xTemp_Struct.bThresholdSet = true;
  xParam.xHum_Struct.bValueSet = false;


  int iRes1, iRes2, iRes3, iRes4;
  iRes1 = iRes2 = iRes3 = iRes4 = 0;

  xMutex = xSemaphoreCreateMutex();


  static TimerHandle_t xOneShotTimer1;

  xOneShotTimer1 = xTimerCreate("Ledon", pdMS_TO_TICKS(3000), pdFALSE,	0, vTimerTurnOff );



  if (xOneShotTimer1 == NULL )
	  // timer not created
	  Error_Handler();

  if (xTimerStart(xOneShotTimer1, 0 ) == pdFAIL){
	  vSendToUart("Timer not started \r\n", uart_command);\
	  Error_Handler();
  }






  static task1_params xTask1args;

  xTask1args.pxDhtPolledData = &xParam.xDhtPolledData;
  xTask1args.pxTimer = &xOneShotTimer1;

  if (xMutex != NULL)
  {

    iRes1 = xTaskCreate(vTaskGetDataDHT, "vTaskGetData", 300, &xTask1args, 4, NULL);
    iRes4 = xTaskCreate(vSendDataThingSpeakTask, "SendDataThingSpeak", 500, &xParam.xDhtPolledData, 3, NULL);
    iRes2 = xTaskCreate(vControlTempHum, "controlTemp", 300, &xParam, 2, NULL);
    iRes3 = xTaskCreate(vRefreshWebserverTask, "RefreshWebserver", 1450, &xParam, 1, NULL);
  }

  /* Check all task were created correctly */
  if (!((iRes1 == pdPASS) && (iRes2 == pdPASS) && (iRes3 == pdPASS) && (iRes4 == pdPASS)))
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
