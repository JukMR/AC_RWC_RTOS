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

/* Standart libraries includes */
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include <stdlib.h>

/* RTOS Includes */
#ifndef RTOS_H
#include "FreeRTOS.h"
#endif
#include "task.h"
#include "semphr.h"
#include "timers.h"

/* Include BSP layer */
#include "bsp.h"

#include "customTypes.h"

#define DEBUG 0

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define wifi_uart &huart1
#define uart_command &huart2

void *vLedRed;
void *vLedBlue;
void *vLedOrange;
void *vLedGreen;

SemaphoreHandle_t xMutex;

/* Struct define */
typedef struct {
	DHT_DataTypedef *pxDhtPolledData;
	TimerHandle_t *pxTimer;
} xTask_params_t;



/* Functions declarations */
void vTaskSendDataThingSpeak(void *pvParameters)
{

	uint8_t uBuffer[2];
	xTask_params_t *pxTmp;

	volatile UBaseType_t uxHighWaterMark;

	for (;;)
	{
		vTurnLedOn(vLedOrange);

		pxTmp = (xTask_params_t *)pvParameters;

		uBuffer[0] = pxTmp->pxDhtPolledData->uTemperature;
		uBuffer[1] = pxTmp->pxDhtPolledData->uHumidity;

		vLogDataThingSpeaker("U6123BFR6YNW5I4V", 2, uBuffer);


		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		if (uxHighWaterMark < 150 || uxHighWaterMark > 250)
			Error_Handler();

		if (xTimerStart(*pxTmp->pxTimer, pdMS_TO_TICKS(1500)) == pdFAIL)
			/* Timer not initialized */
			Error_Handler();


		/* wait 15s between sends */
		vTaskDelay(pdMS_TO_TICKS(15000));
	}
}


void vTaskRefreshWebserver(void *pvParameters)
{
	static xRefreshWebServer_t *xParameters;

	volatile UBaseType_t uxHighWaterMark;

	for (;;)
	{

		xParameters = (xRefreshWebServer_t *)pvParameters;

		vRefreshWebserver(xParameters->control, xParameters->xSharedArgs);

		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		if (uxHighWaterMark < 50 || uxHighWaterMark > 150)
			Error_Handler();

		vTaskDelay(pdMS_TO_TICKS(500));

	}
}


void vTaskControlTempHum(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	ControlTempParams_t *xParam;

	volatile UBaseType_t uxHighWaterMark;

	for (;;)
	{

		xParam = (ControlTempParams_t *)pvParameters;

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

		// Temperature value set activated
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


		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		if (uxHighWaterMark < 150)
			Error_Handler();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(6000));
	}
}


void vTaskGetDataDHT(void *pvParameters)
{

	char buf[30];
	TickType_t xLastWakeTime = xTaskGetTickCount();

	xTask_params_t *xVar;

	volatile UBaseType_t uxHighWaterMark;

	for (;;)
	{
		vTurnLedOn(vLedBlue);

		xVar = (xTask_params_t *)pvParameters;

		vReadDHTSensor(xVar->pxDhtPolledData);



		/* Log data to uart
		 *
		 * These functions could be erased. Debug use only
		 */
		#if DEBUG
		sprintf(buf, "Temp: %u, Hum:%u\r\n", xVar->pxDhtPolledData->uTemperature, xVar->pxDhtPolledData->uHumidity);
		vSendToUart(buf, uart_command);
		#endif

		//xSemaphoreGive(xMutex);

		/* Check stack size is enough */
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		if (uxHighWaterMark < 150)
			Error_Handler();

		if (xTimerStart(*xVar->pxTimer, pdMS_TO_TICKS(1500)) == pdFAIL)
		{
			/* Timer not initialized */
			Error_Handler();
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));
	}
}


void HandleScheduledCommand(char *pcCommand, char *pcArg1, char *pcArg2){
	if (!(strcmp(pcCommand, "turnOff"))){
		vSendToUart("do=turnOff.\r\n", uart_command);
	}

	else if (!(strcmp(pcCommand, "turnOn"))){
		vSendToUart("do=turnOn.\r\n", uart_command);
	}

	else if (!(strcmp(pcCommand, "setTemp"))){
		char buf[32];
		uint8_t uArg1 = atoi(pcArg1);
		uint8_t uArg2 = atoi(pcArg2);
		sprintf(buf, "do=setTemp,%u,%u.\r\n", uArg1, uArg2);
		vSendToUart(buf, uart_command);
	}

	else if (!(strcmp(pcCommand, "setHum"))){
		char buf[32];
		uint8_t uArg1 = atoi(pcArg1);
		uint8_t uArg2 = atoi(pcArg2);
		sprintf(buf, "do=setHum,%u,%u.\r\n", uArg1, uArg2);
		vSendToUart(buf, uart_command);
	}

	else if (!(strcmp(pcCommand, "setRangeTemp"))){
		char buf[32];
		uint8_t uArg1 = atoi(pcArg1);
		uint8_t uArg2 = atoi(pcArg2);
		sprintf(buf, "do=setRangeTemp,%u,%u.\r\n", uArg1, uArg2);
		vSendToUart(buf, uart_command);
	}

	else if (!(strcmp(pcCommand, "setRangeHum"))){
		char buf[32];
		uint8_t uArg1 = atoi(pcArg1);
		uint8_t uArg2 = atoi(pcArg2);
		sprintf(buf, "do=setRangeHum,%u,%u.\r\n", uArg1, uArg2);
		vSendToUart(buf, uart_command);
	}

	else if (!(strcmp(pcCommand, "special1"))){
		vSendToUart("do=special1.\r\n", uart_command);
	}

	else if (!(strcmp(pcCommand, "special2"))){
		vSendToUart("do=special2.\r\n", uart_command);
	}

	else if (!(strcmp(pcCommand, "special3"))){
		vSendToUart("do=special3.\r\n", uart_command);
	}
}


void vScheduleTask(void * pvParameters){
	xScheduledTask_t *cast;

	TickType_t xLastWakeTime = xTaskGetTickCount();

	{
		cast = (xScheduledTask_t *) pvParameters;

		if (cast->time == 0){
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1 * 1000));
		} else {
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(cast->time * 1000));
		}

		HandleScheduledCommand(cast->command, cast->arg1, cast->arg2);

	}
		vTaskDelete(NULL);
		/* Function should not reach here */
		Error_Handler();
	}

void vTaskCreateTimer(void *pvParameters){
	BaseType_t taskCreated;
	TaskHandle_t xHandle = NULL;
	xScheduledTask_t *cast ;
	xScheduledTask_t tmp = {0};

	for (;;) {

		xSemaphoreTake(xMutex, portMAX_DELAY);

		cast = (xScheduledTask_t *) pvParameters;
		strcpy(&(tmp.command), cast->command);
		strcpy(&(tmp.arg1), cast->arg1);
		strcpy(&(tmp.arg2), cast->arg2);
		tmp.time = cast->time;

		taskCreated = xTaskCreate(vScheduleTask, "Demo", 200, &tmp, 3, &xHandle);

		if (taskCreated == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY){
			/* Task not created */
			Error_Handler();
		}

	}
}


/* Timer Callback function */
void prvTimerCallbackHandler(TimerHandle_t xTimer)
{
	uint32_t uTimerID;

	uTimerID = ( uint32_t ) pvTimerGetTimerID( xTimer );

	if      (uTimerID == 1)	    vTurnLedOff(vLedBlue);
	else if (uTimerID == 2)	    vTurnLedOff(vLedOrange);
}


int main(void)
{
	BSP_Init();

	vConnectWifi_StaticIp("Diagon Alley 2.4GHz", "hayunboggartenlaalacena", "192.168.0.200");

	/* Initialize some params */

	static ControlTempParams_t xParam;

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

	int iRes1, iRes2, iRes3, iRes4, iRes5;
	iRes1 = iRes2 = iRes3 = iRes4 = iRes5 = 0;

	xMutex = xSemaphoreCreateBinary();
	// Check out this
	xSemaphoreTake(xMutex, 0);

	/* Start timers */
	static TimerHandle_t xBlinkBlueLed, xBlinkOrangeLed;

	xBlinkBlueLed = xTimerCreate("Ledoff_DHT", pdMS_TO_TICKS(1000), pdFALSE, (void *) 1, prvTimerCallbackHandler);
	xBlinkOrangeLed = xTimerCreate("Ledoff_ThingSpeak_logger", pdMS_TO_TICKS(1000), pdFALSE, (void *) 2, prvTimerCallbackHandler);

	/* Check timers were created correctly */
	if (xBlinkBlueLed == NULL || xBlinkOrangeLed == NULL)
		// timer not created
		Error_Handler();

	/* Check timers were started correctly */
	if (xTimerStart(xBlinkBlueLed, 0) == pdFAIL || xTimerStart(xBlinkOrangeLed, 0) == pdFAIL)
	{
		vSendToUart("Timer not started \r\n", uart_command);
		Error_Handler();
	}


	/* Start static variables */
	static xTask_params_t xTask1Args, xTask2Args;

	xTask1Args.pxDhtPolledData = &xParam.xDhtPolledData;
	xTask1Args.pxTimer = &xBlinkBlueLed;

	xTask2Args.pxDhtPolledData = &xParam.xDhtPolledData;
	xTask2Args.pxTimer = &xBlinkOrangeLed;

	/* Start SchedulerTask variables */
	xRefreshWebServer_t *xRefreshVar = calloc(1, sizeof(xRefreshWebServer_t));
	xScheduledTask_t *xSharedArgs = calloc(1, sizeof(xScheduledTask_t));


	strcpy(xSharedArgs->command, "turnOff");
	strcpy(xSharedArgs->arg1, "23");
	strcpy(xSharedArgs->arg2, "32");
	xSharedArgs->time = 5u;

	xRefreshVar->xSharedArgs = xSharedArgs;
	xRefreshVar->control = &xParam;


	if (xMutex != NULL)
	{
		iRes1 = xTaskCreate(vTaskGetDataDHT, "vTaskGetData", 300, &xTask1Args, 4, NULL);
		iRes2 = xTaskCreate(vTaskSendDataThingSpeak, "vTaskSendDataThingSpeak", 500, &xTask2Args, 3, NULL);
		iRes3 = xTaskCreate(vTaskControlTempHum, "vTaskControlTempHum", 300, &xParam, 2, NULL);
		iRes4 = xTaskCreate(vTaskRefreshWebserver, "RefreshWebserver", 1450, xRefreshVar, 1, NULL);
		iRes5 = xTaskCreate(vTaskCreateTimer, "vCreateTimerTask", 300, xSharedArgs, 3, NULL);
	}

	/* Check all task were created correctly */
	if (!((iRes1 == pdPASS) && (iRes4 == pdPASS) && (iRes3 == pdPASS) && (iRes4 == pdPASS )&& (iRes5 == pdPASS)))
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
