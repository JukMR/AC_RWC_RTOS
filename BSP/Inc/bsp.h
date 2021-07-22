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

typedef struct
{
	uint8_t Temperature;
	uint8_t Humidity;
}DHT_DataTypedef;

typedef struct
{
	char minTemp[8];
	char maxTemp[8];
	char minHum[8];
	char maxHum[8];
}controlData;

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


void vSetTemp(int Value);

void vSetHum(int Value);


void vTurnOn(void);

void vTurnOff(void);

void SystemClock_Config(void);

void Error_Handler(void);

void BSP_Init(void);

void vConnectWifi_StaticIp(char *ssid, char *pass, char *ip);

void RefreshWebserver(ControlTempParams *arg);

void readDHTSensor(DHT_DataTypedef *dataStruct);

#endif /* INC_Bsp_H_ */

