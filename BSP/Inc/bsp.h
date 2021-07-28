/*
 * Bsp.h
 *
 *  Created on: Jul 22, 2021
 *      Author: Julian
 */

#ifndef BSP__H_
#define BSP__H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

#include "customTypes.h"

typedef struct
{
	char pcMinTemp[ 8 ];
	char pcMaxTemp[ 8 ];
	char pcMinHum[ 8 ];
	char pcMaxHum[ 8 ];
} xSetRangeBuffer_t;

typedef struct
{
  bool bValueSet;
  uint8_t uValue;
  bool bThresholdSet;
  uint8_t uMin;
  uint8_t uMax;
} xStateMode_t;

typedef struct
{
  DhtReadings_t xDhtPolledData;
  xStateMode_t xTemp_Struct;
  xStateMode_t xHum_Struct;
} xStateStructure_t;

typedef struct
{
  char pcCommand[ 16 ];
  char pcDays[ 16 ];
  char pcHours[ 16 ];
  char pcMinutes[ 16 ];
  char pcSeconds[ 16 ];
  char pcArg1[ 16 ];
  char pcArg2[ 16 ];
} xTimerBuffer_t;

typedef struct {
	xStateStructure_t *pxControl;
	xDelayTask_t *pxSharedArgs;
} xRefreshWebServer_t;



void vSetTemp(uint8_t Value);

void vSetHum(uint8_t Value);

void vTurnOn(void);

void vTurnOff(void);

void SystemClock_Config(void);

void Error_Handler(void);

void BSP_Init(void);

void vConnectWifi_StaticIp(char *ssid, char *pass, char *ip);

void vRefreshWebserver(xStateStructure_t *arg, xDelayTask_t *xSharedArgs);

void vReadDHTSensor(DhtReadings_t *dataStruct);

void vLogDataThingSpeaker(char *api, int number_values, uint8_t *value);

void vSendToUart(const char *str, UART_HandleTypeDef *uart);

void vTurnLedOn(void *led);

void vTurnLedOff(void *led);

#endif /* INC_Bsp_H_ */

