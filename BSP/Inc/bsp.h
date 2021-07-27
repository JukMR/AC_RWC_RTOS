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
	char pcMinTemp[8];
	char pcMaxTemp[8];
	char pcMinHum[8];
	char pcMaxHum[8];
}controlData;

typedef struct
{
  bool bValueSet;
  uint8_t uValue;
  bool bThresholdSet;
  uint8_t uMin;
  uint8_t uMax;
} Threshold_TypeDef;

typedef struct
{
  DHT_DataTypedef xDhtPolledData;
  Threshold_TypeDef xTemp_Struct;
  Threshold_TypeDef xHum_Struct;
} ControlTempParams_t;

typedef struct
{
  char command[ 16 ];
  char days[ 16 ];
  char hours[ 16 ];
  char minutes[ 16 ];
  char seconds[ 16 ];
  char param1[ 16 ];
  char param2[ 16 ];

}xScheduledTaskParams_t;

typedef struct {
	ControlTempParams_t *control;
	xScheduledTask_t *xSharedArgs;
} xRefreshWebServer_t;



void vSetTemp(uint8_t Value);

void vSetHum(uint8_t Value);

void vTurnOn(void);

void vTurnOff(void);

void SystemClock_Config(void);

void Error_Handler(void);

void BSP_Init(void);

void vConnectWifi_StaticIp(char *ssid, char *pass, char *ip);

void vRefreshWebserver(ControlTempParams_t *arg, xScheduledTask_t *xSharedArgs);

void vReadDHTSensor(DHT_DataTypedef *dataStruct);

void vLogDataThingSpeaker(char *api, int number_values, uint8_t *value);

void vSendToUart(const char *str, UART_HandleTypeDef *uart);

void vTurnLedOn(void *led);

void vTurnLedOff(void *led);

#endif /* INC_Bsp_H_ */

