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

#define DEBUG 1

extern UART_HandleTypeDef huart2;

#define uart_command &huart2

void *vLedRed;
void *vLedBlue;
void *vLedOrange;
void *vLedGreen;

SemaphoreHandle_t xSemaphoreOneShotTask, xMutexEsp8266;
QueueHandle_t xDhtQueue;

/* Struct define */
typedef struct {
	DhtReadings_t *pxDhtPolledData;
	TimerHandle_t *pxTimer;
} xTask_params_t;



/* Functions declarations */
void vTaskSendDataThingSpeak( void *pvParameters )
{

	uint8_t uBuffer[2] = { 0 };
	xTask_params_t *pxTmp;

	#if DEBUG
	volatile UBaseType_t uxHighWaterMark;
	#endif

	for (;;)
	{
		vTurnLedOn( vLedOrange );

		pxTmp = ( xTask_params_t * ) pvParameters;

		uBuffer[0] = pxTmp->pxDhtPolledData->uTemperature;
		uBuffer[1] = pxTmp->pxDhtPolledData->uHumidity;
		vLogDataThingSpeaker( "U6123BFR6YNW5I4V", 2, uBuffer );

		#if DEBUG
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		if ( uxHighWaterMark < 100 || uxHighWaterMark > 250 )
			Error_Handler();
		#endif

		if ( xTimerStart( *pxTmp->pxTimer, pdMS_TO_TICKS( 1500 ) ) == pdFAIL )
			/* Timer not initialized */
			Error_Handler();

		/* wait 15s between sends */
		vTaskDelay( pdMS_TO_TICKS( 15000 ) );
	}
}


void vTaskRefreshWebserver( void *pvParameters )
{
	static xRefreshWebServer_t *xParameters;

	#if DEBUG
	volatile UBaseType_t uxHighWaterMark;
	#endif

	for (;;)
	{
		xParameters = ( xRefreshWebServer_t * ) pvParameters;

		vRefreshWebserver( xParameters->pxControl, xParameters->pxSharedArgs );

		#if DEBUG
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		if ( uxHighWaterMark < 50 )
			Error_Handler();
		#endif

		vTaskDelay( pdMS_TO_TICKS( 100 ) );
	}
}


void vTaskControlTempHum( void *pvParameters )
{
	xStateStructure_t *xParam;

	#if DEBUG
	volatile UBaseType_t uxHighWaterMark;
	#endif

	for (;;)
	{
		xParam = ( xStateStructure_t * ) pvParameters;

		DhtReadings_t tmp = { 0 };

		xQueueReceive( xDhtQueue, &tmp, portMAX_DELAY );

		/* Save temperature and humidity value to memory */
		xParam->xDhtPolledData.uTemperature = tmp.uTemperature;
		xParam->xDhtPolledData.uHumidity = tmp.uHumidity;

		/* Temperature Control */
		/* Threshold activated */
		if ( xParam->xTemp_Struct.bThresholdSet )
		{
			/* temperature is lower than threshold */
			if ( xParam->xDhtPolledData.uTemperature < xParam->xTemp_Struct.uMin )
			{
				vSetTemp( xParam->xTemp_Struct.uMax );
			}
			/* temperature is higher than threshold */
			else if ( xParam->xDhtPolledData.uTemperature > xParam->xTemp_Struct.uMax )
			{
				vSetTemp( xParam->xTemp_Struct.uMin );
			}

		/* Temperature value set activated */
		}
		else if ( xParam->xTemp_Struct.bValueSet )
		{
			if ( xParam->xDhtPolledData.uTemperature != xParam->xTemp_Struct.uValue )
			{
				vSetTemp( xParam->xTemp_Struct.uValue );
			}
		}

		/* Humidity Control */
		/* Threshold activated */
		if ( xParam->xHum_Struct.bThresholdSet )
		{

			/* temperature is lower than threshold */
			if ( xParam->xDhtPolledData.uHumidity < xParam->xHum_Struct.uMin )
			{
				vSetHum( xParam->xHum_Struct.uMax );
			}

			/* temperature is higher than threshold */
			else if ( xParam->xDhtPolledData.uHumidity > xParam->xHum_Struct.uMax )
			{
				vSetHum( xParam->xHum_Struct.uMin );
			}

		/* temperature value set */
		}
		else if ( xParam->xHum_Struct.bValueSet )
		{
			if ( xParam->xDhtPolledData.uHumidity != xParam->xHum_Struct.uValue )
			{
				vSetHum( xParam->xHum_Struct.uValue );
			}
		}


		#if DEBUG
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		if ( uxHighWaterMark < 50 || uxHighWaterMark > 250 )
			Error_Handler();
		#endif
	}
}


void vTaskGetDataDHT( void *pvParameters )
{

	char buf[ 30 ];
	TickType_t xLastWakeTime = xTaskGetTickCount();

	xTimerHandle *xTimer;

	DhtReadings_t tmp = { 0 };

	#if DEBUG
	volatile UBaseType_t uxHighWaterMark;
	#endif

	for (;;)
	{
		vTurnLedOn( vLedBlue );

		xTimer = ( xTimerHandle * ) pvParameters;

		vReadDHTSensor( &tmp );

		xQueueSend( xDhtQueue, &tmp, portMAX_DELAY );

		#if DEBUG
		/* Log data to uart
		 *
		 * These functions could be erased. Debug use only
		 */
		sprintf( buf, "Temp: %u, Hum:%u\r\n", tmp.uTemperature, tmp.uHumidity );
		vSendToUart( buf, uart_command );
		#endif


		#if DEBUG
		/* Check stack size is enough */
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		if ( uxHighWaterMark < 150 || uxHighWaterMark > 250 )
			Error_Handler();
		#endif

		if ( xTimerStart( *xTimer, pdMS_TO_TICKS( 1500 ) ) == pdFAIL )
		{
			/* Timer not initialized */
			Error_Handler();
		}

		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 3000 ) );
	}
}


void HandleScheduledCommand( char *pcCommand, char *pcArg1, char *pcArg2 ){

	if ( !(strcmp( pcCommand, "turnOff" )) ) {
		vSendToUart( "do=turnOff.\r\n", uart_command );
	}

	else if ( !( strcmp( pcCommand, "turnOn" ) ) ) {
		vSendToUart( "do=turnOn.\r\n", uart_command );
	}

	else if ( !( strcmp( pcCommand, "setTemp" ) ) ) {
		char buf[ 32 ];
		uint8_t uArg1 = atoi( pcArg1 );
		uint8_t uArg2 = atoi( pcArg2 );
		sprintf( buf, "do=setTemp,%u,%u.\r\n", uArg1, uArg2 );
		vSendToUart( buf, uart_command );
	}

	else if ( !( strcmp( pcCommand, "setHum" ) ) ) {
		char buf[ 32 ];
		uint8_t uArg1 = atoi( pcArg1 );
		uint8_t uArg2 = atoi( pcArg2 );
		sprintf( buf, "do=setHum,%u,%u.\r\n", uArg1, uArg2 );
		vSendToUart( buf, uart_command );
	}

	else if ( !( strcmp( pcCommand, "setRangeTemp" ) ) ) {
		char buf[ 32 ];
		uint8_t uArg1 = atoi( pcArg1 );
		uint8_t uArg2 = atoi( pcArg2 );
		sprintf( buf, "do=setRangeTemp,%u,%u.\r\n", uArg1, uArg2 );
		vSendToUart( buf, uart_command );
	}

	else if ( !( strcmp( pcCommand, "setRangeHum" ) ) ) {
		char buf[ 32 ];
		uint8_t uArg1 = atoi( pcArg1 );
		uint8_t uArg2 = atoi( pcArg2 );
		sprintf( buf, "do=setRangeHum,%u,%u.\r\n", uArg1, uArg2 );
		vSendToUart( buf, uart_command );
	}

	else if ( !( strcmp( pcCommand, "special1" ) ) ) {
		char buf[ 64 ];
		sprintf( buf, "do=special1,%s,%s.\r\n", pcArg1, pcArg2 );
		vSendToUart( buf, uart_command );
	}

	else if ( !( strcmp( pcCommand, "special2" ) ) ) {
		char buf[ 64 ];
		sprintf( buf, "do=special2,%s,%s.\r\n", pcArg1, pcArg2 );
		vSendToUart( buf, uart_command );
	}

	else if ( !( strcmp( pcCommand, "special3" ) ) ) {
		char buf[ 64 ];
		sprintf( buf, "do=special3,%s,%s.\r\n", pcArg1, pcArg2 );
		vSendToUart( buf, uart_command );
	}
}


void vDelayTask( void * pvParameters ){

	TickType_t xLastWakeTime = xTaskGetTickCount();
	xDelayTask_t *cast = ( xDelayTask_t * ) pvParameters;
	xDelayTask_t tmp = { 0 };

	strcpy( (char *) &(tmp.pcCommand) , &(*cast->pcCommand));
	strcpy( (char *) &(tmp.pcArg1) , &(*cast->pcArg1));
	strcpy( (char *) &(tmp.pcArg2) , &(*cast->pcArg2));
	tmp.uTime = cast->uTime;

	if ( tmp.uTime == 0 )
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1 * 1000 ) );
	else
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( tmp.uTime * 1000 ) );

	/* Send delayed command */
	HandleScheduledCommand( (char *) &(tmp.pcCommand), (char *) &(tmp.pcArg1) , (char *) &(tmp.pcArg2) );

	vTaskDelete( NULL );

	/* Task should not reach here */
	Error_Handler();
}


void vTaskDelayedCommand( void *pvParameters ){
	BaseType_t taskCreated;
	TaskHandle_t xHandle = NULL;
	xDelayTask_t *cast;

	for (;;) {
		xSemaphoreTake ( xSemaphoreOneShotTask, portMAX_DELAY );

		cast = ( xDelayTask_t * ) pvParameters;

		taskCreated = xTaskCreate( vDelayTask, "vDelayTask", 200, cast, 3, &xHandle );

		if ( taskCreated == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY ){
			/* Task not created */
			Error_Handler();
		}
	}
}


/* Timer Callback function */
void prvTimerCallbackHandler( TimerHandle_t xTimer )
{
	uint32_t uTimerID;
	uTimerID = ( uint32_t ) pvTimerGetTimerID( xTimer );

	if      ( uTimerID == 1 )	    vTurnLedOff( vLedBlue );
	else if ( uTimerID == 2 )	    vTurnLedOff( vLedOrange );
}


xStateStructure_t *initializeParams( void ) {
	xStateStructure_t *xParam = calloc( 1, sizeof( xStateStructure_t ) );

	xParam->xDhtPolledData.uTemperature = 0;
	xParam->xDhtPolledData.uHumidity = 5;

	xParam->xTemp_Struct.uMin = 0;
	xParam->xTemp_Struct.uMax = 50;
	xParam->xTemp_Struct.bThresholdSet = false;
	xParam->xTemp_Struct.bValueSet = false;

	xParam->xHum_Struct.uMin = 5;
	xParam->xHum_Struct.uMax = 95;
	xParam->xTemp_Struct.bThresholdSet = false;
	xParam->xHum_Struct.bValueSet = false;

	return xParam;
}


int main( void )
{
	BSP_Init();

	vConnectWifi_StaticIp( "Diagon Alley 2.4GHz", "hayunboggartenlaalacena", "192.168.0.200" );

	/* Initialize some params */
	xStateStructure_t *xParam = initializeParams();


	/* Create OneShotTask Semaphore */
	xSemaphoreOneShotTask = xSemaphoreCreateBinary();


	/* Initialize the semaphore as taken */
	xSemaphoreTake( xSemaphoreOneShotTask, 0 );


	/* Start timers */
	static TimerHandle_t xBlinkBlueLed, xBlinkOrangeLed;

	xBlinkBlueLed = xTimerCreate( "Ledoff_DHT", pdMS_TO_TICKS( 1000 ), pdFALSE, ( void * ) 1, prvTimerCallbackHandler );
	xBlinkOrangeLed = xTimerCreate( "Ledoff_ThingSpeak_logger", pdMS_TO_TICKS( 1000 ), pdFALSE, ( void * ) 2, prvTimerCallbackHandler );


	/* Check timers were created correctly */
	if ( xBlinkBlueLed == NULL || xBlinkOrangeLed == NULL )
		/* timer not created */
		Error_Handler();


	/* Check timers were started correctly */
	if ( xTimerStart( xBlinkBlueLed, 0 ) == pdFAIL || xTimerStart( xBlinkOrangeLed, 0 ) == pdFAIL )
	{
		vSendToUart( "Timer not started \r\n", uart_command );
		Error_Handler();
	}


	/* Start static variables */
	xTask_params_t *pxTask2Args = calloc( 1, sizeof( xTask_params_t ) );

	pxTask2Args->pxDhtPolledData = &xParam->xDhtPolledData;
	pxTask2Args->pxTimer = &xBlinkOrangeLed;


	/* Start SchedulerTask variables */
	xDelayTask_t *pxSharedArgs = calloc( 1, sizeof( xDelayTask_t ) );

	xRefreshWebServer_t *pxRefreshVar = calloc( 1, sizeof( xRefreshWebServer_t ) );
	pxRefreshVar->pxSharedArgs = pxSharedArgs;
	pxRefreshVar->pxControl = xParam;


	/* Initialize Queue */
	xDhtQueue = xQueueCreate( 1, sizeof( DhtReadings_t ) );


	int iRes1 = 0, iRes2 = 0, iRes3 = 0, iRes4 = 0, iRes5 = 0;

	if ( xSemaphoreOneShotTask != NULL )
	{
		iRes1 = xTaskCreate( vTaskGetDataDHT, "vTaskGetDataDHT", 300, (void *) &xBlinkBlueLed, 5, NULL );
		iRes2 = xTaskCreate( vTaskSendDataThingSpeak, "vTaskSendDataThingSpeak", 470, (void *) pxTask2Args, 3, NULL );
		iRes3 = xTaskCreate( vTaskControlTempHum, "vTaskControlTempHum", 215, (void *) pxRefreshVar->pxControl, 4, NULL );
		iRes4 = xTaskCreate( vTaskRefreshWebserver, "vTaskRefreshWebserver", 1450, (void *) pxRefreshVar, 1, NULL );
		iRes5 = xTaskCreate( vTaskDelayedCommand, "vTaskDelayedCommand", 300, (void *) pxRefreshVar->pxSharedArgs, 2, NULL );
	}

	/* Check all task were created correctly */
	if ( ! ( ( iRes1 == pdPASS ) && ( iRes2 == pdPASS ) && ( iRes3 == pdPASS ) && ( iRes4 == pdPASS ) && ( iRes5 == pdPASS ) ) )
	{
		Error_Handler();
	}

	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	for (;;)
		Error_Handler();
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
