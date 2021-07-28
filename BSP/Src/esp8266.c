/**
  ******************************************************************************

  File:		ESP DATA HANDLER
  Author:   ControllersTech
  Updated:  3rd Aug 2020

  ******************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ******************************************************************************
*/

#include <esp8266.h>
#include "UartRingbuffer.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"

#ifndef RTOS_H
#include "FreeRTOS.h"
#endif
#include "semphr.h"

#include "customTypes.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define wifi_uart &huart1
#define device_uart &huart2

#define WAIT_TIMEOUT 3
#define WAIT_OK 1
#define WAIT_ERROR 2

extern SemaphoreHandle_t xSemaphoreOneShotTask;

// char buffer[64];

/* Start of main page */
char *home_header =
	"<!DOCTYPE html>\n\
<html lang=\'en\'>\n\
<head>\n\
    <meta charset=\"UTF 8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <title Servidor Esp8266></title>\n\
</head>\n\
<body>\n\
    <div style=\"text-align: center;\">\n\
        <h1> Servidor Esp8266 con DHT11</h1>\n\
        <div>\n\
            <h2> Encendido/Apagado</h2>\n\
            <button onclick=location.href=\"turnon\"> Encender AC </button>\n\
            &nbsp;&nbsp;\n\
            <button onclick=location.href=\"turnoff\"> Apagar AC </button>\n\
        </div>";



char *home_tail =

       "<form action=\"/setForm\">\n\
            <h2> Elegir temperatura </h2>\n\
            <label for=\"temp\"> Temperatura: </label>\n\
            <input type=\"text\" id=\"idSetTemp\" name=\"setTemp\" value=\"\"> <br> </br>\n\
            <h2> Elegir humedad </h2>\n\
            <label for=\"hum\"> Humedad: </label>\n\
            <input type=\"text\" id=\"idSetHum\" name=\"setHum\" value=\"\"> <br> </br>\n\
            <input type=\"submit\" value=\"Set Values\">\n\
        </form>\n\
        <h2>Elegir rangos</h2>\n\
        <form action=\"/setRange\">\n\
            <p> Temperature Range </p>\n\
            <i>(Value between 0 and 50)</i><br></br>\n\
            <label for=\"minTemp\">Minimum:</label>\n\
            <input type=\"text\" id=\"minTemp\" name=\"minTemp\" value=\"\"><br></br>\n\
            <label for=\"maxTemp\">Maximum:</label>\n\
            <input type=\"text\" id=\"maxTemp\" name=\"maxTemp\" value=\"\"><br><br>\n\
            <p> Humidity Range </p>\n\
            <i>(Value between 5 and 95)</i><br></br>\n\
            <label for=\"minHum\">Minimum:</label>\n\
            <input type=\"text\" id=\"minHum\" name=\"minHum\" value=\"\"><br></br>\n\
            <label for=\"maxHum\">Maximum:</label>\n\
            <input type=\"text\" id=\"maxHum\" name=\"maxHum\" value=\"\"><br><br>\n\
            <input type=\"submit\" value=\"Set Ranges\">\n\
        </form><br><br>\n\
        <button onclick=location.href=\"https://thingspeak.com/channels/1439978\"> Watch thingSpeak data</button> <br>\n\
        <br>";

char *specialCommand =
        "<div>\n\
		    <h2>Select one of the special command</h2>\n\
            <form action=\"/special\">\n\
                <select name=\"com\" id=\"com\">\n\
                    <option value=\"special1\">special1</option>\n\
                    <option value=\"special2\">special2</option>\n\
                    <option value=\"special3\">special3</option>\n\
                </select>\n\
                <br></br>\n\
                <label for=\"arg1\">Arg1:</label>\n\
                <input type=\"text\" id=\"arg1\" name=\"arg1\" value=\"\"><br></br>\n\
                <label for=\"arg2\">Arg2:</label>\n\
                <input type=\"text\" id=\"arg2\" name=\"arg2\" value=\"\"><br><br>\n\
                <input type=\"submit\" value=\"Send special\">\n\
            </form>\n\
            <br></br>\n\
        </div>";


char *scheduledCommands =
   "<div>\n\
        <h2> Program a command to be executed in the future </h2>\n\
        <form action=\"timer\">\n\
        <label for=\"com\">Choose a command:</label>\n\
        <select name=\"com\" id=\"com\">\n\
            <option value=\"turnOn\">turnOn</option>\n\
            <option value=\"turnOff\">turnOff</option>\n\
            <option value=\"setTemp\">setTemp</option>\n\
            <option value=\"setHum\">setHum</option>\n\
            <option value=\"setRangeTemp\">setRangeTemp</option>\n\
            <option value=\"setRangeHum\">setRangeHum</option>\n\
            <option value=\"special1\">special1</option>\n\
            <option value=\"special2\">special2</option>\n\
            <option value=\"special3\">special3</option>\n\
        </select>\n\
        <br></br>\n\
        <label for=\"days\">Insert days of delay</label>\n\
        <input type=\"text\" id=\"days\" name=\"days\" value=\"\"><br><br>\n\
        <label for=\"hs\">Insert hours of delay</label>\n\
        <input type=\"text\" id=\"hs\" name=\"hs\" value=\"\"><br><br>\n\
        <label for=\"min\">Insert minutes of delay</label>\n\
        <input type=\"text\" id=\"min\" name=\"min\" value=\"\"><br><br>\n\
        <label for=\"sec\">Insert seconds of delay</label>\n\
        <input type=\"text\" id=\"sec\" name=\"sec\" value=\"\"><br><br>\n\
        <label for=\"arg\">Insert args here if the action requires it:</label>\n\
        <br><br>\n\
        <input type=\"text\" id=\"arg1\" name=\"arg1\" value=\"\">\n\
        <input type=\"text\" id=\"arg2\" name=\"arg2\" value=\"\">\n\
        <input type=\"submit\" value=\"Queue command \">\n\
    </form>\n\
    </div>\n\
</div>\n\
</body>\n\
</html>";

/* end of main page */

/* Receive command page */
char *receiveCommand =
	"<!DOCTYPE html>\n\
<html>\n\
<body>\n\
    <div style=\"text-align: center;\">\n\
        <h1 style=\"text-align: center;\"> Servidor Esp8266 con DHT11</h1>\n\
        <h2> Command send successfully </h2>\n\
        <p> Click below to return the main page</p>\n\
        <button onclick=location.href=\"main\"> Go back </button>\n\
    </div>\n\
</body>\n\
</html>";

/* End of receive command page */

/* Error page */
char *error =
	"<!DOCTYPE html>\n\
			<html>\n\
			<body>\n\
			    <div>\n\
			        <p>\n\
			            Error\n\
			        </p>\n\
			    </div>\n\
			</body>\n\
			</html>";
/* End of error page */


/***********************************************************************/

int wait_timeout(char *str, UART_HandleTypeDef *uart, uint32_t times);

void ESP_Init(char *SSID, char *PASSWD, char *STAIP)
{
	char data[96];

	Ringbuf_init();

	Uart_sendstring("AT+RST\r\n", wifi_uart);
	HAL_Delay(2000);

	/********* AT **********/
	Uart_flush(wifi_uart);
	Uart_sendstring("AT\r\n", wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/********* AT+CWMODE=1 **********/
	Uart_flush(wifi_uart);
	Uart_sendstring("AT+CWMODE=1\r\n", wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/* Set Static IP Address */
	/********* AT+CWSTAIP=IPADDRESS **********/
	Uart_flush(wifi_uart);
	sprintf(data, "AT+CIPSTA=\"%s\"\r\n", STAIP);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/********* AT+CWJAP="SSID","PASSWD" **********/
	Uart_flush(wifi_uart);
	sprintf(data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/********* AT+CIPMUX **********/
	Uart_flush(wifi_uart);
	Uart_sendstring("AT+CIPMUX=1\r\n", wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/********* AT+CIPSERVER **********/
	Uart_flush(wifi_uart);
	Uart_sendstring("AT+CIPSERVER=1,80\r\n", wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	Uart_flush(wifi_uart);
	HAL_Delay(200);
}

int Server_Send(char *str, int Link_ID)
{
	int len = strlen(str);
	char data[96];
	Uart_flush(wifi_uart);
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", Link_ID, len);
	Uart_sendstring(data, wifi_uart);


	while (!(Wait_for(">", wifi_uart)));
	Uart_sendstring(str, wifi_uart);

	while (!(Wait_for("SEND OK", wifi_uart)));
	Uart_flush(wifi_uart);
	sprintf(data, "AT+CIPCLOSE=%d\r\n", Link_ID);
	Uart_sendstring(data, wifi_uart);

	while (!(Wait_for("OK\r\n", wifi_uart)));
	return 1;

}


int Server_Send_main(char *start, char *end, char *timers, char *special, int Link_ID, DHT_DataTypedef *dht_struct)
{
	int lenStart = strlen(start);
	char data[80];
	char body[200];

	/* header processing part */
	Uart_flush(wifi_uart);
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenStart);
	Uart_sendstring(data, wifi_uart);

	if (!(wait_timeout(">", wifi_uart, 10000000))) {
		goto reset;
	}

//	while (!(Wait_for(">", wifi_uart)));
	Uart_sendstring(start, wifi_uart);
//	while (!(Wait_for("SEND OK", wifi_uart)));
	if (!(wait_timeout("SEND OK", wifi_uart, 10000000))) {
		goto reset;
	}

	/* param processing part */
	Uart_flush(wifi_uart);

	sprintf(body, "<div><h2>Temperatura actual: %u</h2><h2>Humedad actual es: %u</h2></div>", dht_struct->uTemperature, dht_struct->uHumidity);
	int lenBody = strlen(body);
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenBody);
	Uart_sendstring(data, wifi_uart);


//	while (!(Wait_for(">", wifi_uart)));
	if (!(wait_timeout(">", wifi_uart, 10000000))) goto reset;

	Uart_sendstring(body, wifi_uart);
//	while (!(Wait_for("SEND OK", wifi_uart)));
	if (!(wait_timeout("SEND OK", wifi_uart, 10000000))) goto reset;

	/* end processing part */
	Uart_flush(wifi_uart);
	int lenEnd = strlen(end);
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenEnd);
	Uart_sendstring(data, wifi_uart);

//	while (!(Wait_for(">", wifi_uart)));
	if (!(wait_timeout(">", wifi_uart, 10000000))) goto reset;
	Uart_sendstring(end, wifi_uart);

//	while (!(Wait_for("SEND OK", wifi_uart)));
	if (!(wait_timeout("SEND OK", wifi_uart, 10000000))) goto reset;

	/* special processing part */
	Uart_flush(wifi_uart);
	int lenSpecial = strlen(special);
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenSpecial);
	Uart_sendstring(data, wifi_uart);

//	while (!(Wait_for(">", wifi_uart)));
	if (!(wait_timeout(">", wifi_uart, 10000000))) goto reset;
	Uart_sendstring(special, wifi_uart);

//	while (!(Wait_for("SEND OK", wifi_uart)));
	if (!(wait_timeout("SEND OK", wifi_uart, 10000000))) goto reset;


	/* Timer processing part */
	Uart_flush(wifi_uart);
	int lenTimer = strlen(timers);
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenTimer);
	Uart_sendstring(data, wifi_uart);

//	while (!(Wait_for(">", wifi_uart)));
	if (!(wait_timeout(">", wifi_uart, 10000000))) goto reset;
	Uart_sendstring(timers, wifi_uart);

//	while (!(Wait_for("SEND OK", wifi_uart)));
	if (!(wait_timeout("SEND OK", wifi_uart, 10000000))) goto reset;


	/* Finish the connection */
	Uart_flush(wifi_uart);
	sprintf(data, "AT+CIPCLOSE=%d\r\n", Link_ID);
	Uart_sendstring(data, wifi_uart);


//	while (!(Wait_for("OK\r\n", wifi_uart)));
	if (!(wait_timeout("OK\r\n", wifi_uart, 10000000))) goto reset;

	return 1;
reset:
	Uart_flush(wifi_uart);
	return 1;
}


void Server_Handle(char *str, int Link_ID, ControlTempParams_t *pxArg)
{
	char datatosend[4096] = {0};

	if (!(strcmp(str, "/receiveCommand")))
	{
		sprintf(datatosend, receiveCommand);
		Server_Send(datatosend, Link_ID);
	}

	else if (!(strcmp(str, "/main")))
	{
		Server_Send_main(home_header, home_tail, scheduledCommands, specialCommand, Link_ID, &pxArg->xDhtPolledData);
	}

	else if (!(strcmp(str, "/setForm")))
	{
		sprintf(datatosend, receiveCommand);
		Server_Send(datatosend, Link_ID);
	}

	else if (!(strcmp(str, "/setRange")))
	{
		sprintf(datatosend, receiveCommand);
		Server_Send(datatosend, Link_ID);
	}

	else if (!(strcmp(str, "/timer")))
	{
		sprintf(datatosend, receiveCommand);
		Server_Send(datatosend, Link_ID);
	}

	else if (!(strcmp(str, "/special")))
	{
		sprintf(datatosend, receiveCommand);
		Server_Send(datatosend, Link_ID);
	}


	else
	{

		/* I think we should not reach here */
		Error_Handler();
		sprintf(datatosend, error);
		Server_Send(datatosend, Link_ID);
	}
}


static void strtoIntValue(char *str, uint8_t *value)
{
	if (strcmp(str, "") && str[0] != 'c')
		*value = (uint8_t )atoi(str);
}


static void setValue(char *str, char *value)
{
	char buff[30] = {0};
	sprintf(buff, "%s%s.\r\n", str, value);
	Uart_sendstring(buff, device_uart);
}


static void setRanges(char *str, uint8_t minArg, uint8_t maxArg)
{
	char buff[30] = {0};
	sprintf(buff, "%s%u,%u.\r\n", str, minArg, maxArg);
	Uart_sendstring(buff, device_uart);
}


static void Handle_controlData(controlData *tmp, ControlTempParams_t *pxArg)
{
	uint8_t uMinTemp, uMaxTemp, uMinHum, uMaxHum;
	uMinTemp = uMaxTemp = uMinHum = uMaxHum = 255;

	strtoIntValue(tmp->pcMinTemp, &uMinTemp);
	strtoIntValue(tmp->pcMaxTemp, &uMaxTemp);
	strtoIntValue(tmp->pcMinHum, &uMinHum);
	strtoIntValue(tmp->pcMaxHum, &uMaxHum);


	/* Check temperature range */
	if ((uMinTemp >= 0) && (uMaxTemp <= 50) && (uMinTemp <= uMaxTemp))
	{
		/* call change threshold function */
		pxArg->xTemp_Struct.uMin = uMinTemp;
		pxArg->xTemp_Struct.uMax = uMaxTemp;

		/* Disable setValue if present and enable setRangeTemp*/
		pxArg->xTemp_Struct.bValueSet = false;
		pxArg->xTemp_Struct.bThresholdSet = true;

		/* Send command to uart */
		setRanges("do=setRangeTemp,", uMinTemp, uMaxTemp);

	/* Reset threshold command */
	} else if (tmp->pcMinTemp[0] == 'c' || tmp->pcMaxTemp[0] == 'c'){
		pxArg->xTemp_Struct.bThresholdSet = false;
		pxArg->xTemp_Struct.bValueSet = false;
	}


	/* Check humidity range */
	if ((uMinHum >= 5) && (uMaxHum <= 95) && (uMinHum <= uMaxHum))
	{
		/* call change threshold function */
		pxArg->xHum_Struct.uMin = uMinHum;
		pxArg->xHum_Struct.uMax = uMaxHum;

		/* Disable setValue if present and enable setRangeHum*/
		pxArg->xHum_Struct.bValueSet = false;
		pxArg->xHum_Struct.bThresholdSet = true;


		/* Send command to uart */
		setRanges("do=setRangeHum,", uMinHum, uMaxHum);

	/* Reset threshold command */
	} else if (tmp->pcMinHum[0] == 'c' || tmp->pcMaxHum[0] == 'c'){
		pxArg->xHum_Struct.bThresholdSet = false;
		pxArg->xHum_Struct.bValueSet = false;

	}
}

void HandleScheduleData(xScheduledTaskParams_t *data, xScheduledTask_t *taskData){
	uint32_t timeInSeconds, days, hours, minutes, seconds;
	timeInSeconds = days = hours = minutes = seconds = 0;

	days = (uint32_t) atoi(data->days);
	hours = (uint32_t) atoi(data->hours);
	minutes = (uint32_t) atoi(data->minutes);
	seconds = (uint32_t) atoi(data->seconds);

	timeInSeconds = days * 86400 + hours * 3600 + minutes * 60 + seconds;

	strcpy(taskData->command, data->command);
	strcpy(taskData->arg1, data->param1);
	strcpy(taskData->arg2, data->param2);
	taskData->time = timeInSeconds;
	if( xSemaphoreGive( xSemaphoreOneShotTask ) != pdTRUE ){
		/* Error, cannot give Semaphore */
		Error_Handler();
	}
}

void Server_Start(ControlTempParams_t *arg, xScheduledTask_t *xSharedArgs)
{
//	int res;
	char buftostoreheader[300] = {0};
	char Link_ID;

//	res = Get_after_timeout( "+IPD,", 1, &Link_ID, wifi_uart, (uint32_t) -1 );
//
//	if (res == WAIT_OK){
//		(void)0;
//	} else if (res == WAIT_TIMEOUT){
//		goto timeout;
//	}

	while (!(Get_after("+IPD,", 1, &Link_ID, wifi_uart)));

	Link_ID -= 48;
	while (!(Copy_upto(" HTTP/1.1", buftostoreheader, wifi_uart)));


	/* Values */
	if (Look_for("/setForm", buftostoreheader) == 1)
	{
		char strTemp[5] = {0};
		char strHum[5] = {0};
		GetDataFromBuffer("setTemp=", "&", buftostoreheader, strTemp);
		GetDataFromBuffer("setHum=", "HTTP", buftostoreheader, strHum);

		if (strcmp(strTemp, " ") && ((char)strTemp[0] != '\0') && (char) strTemp[0] != 'c')
		{
			/* Set temp */
			setValue("do=setTemp,", strTemp);
			strtoIntValue(strTemp, &arg->xTemp_Struct.uValue);
			arg->xTemp_Struct.bValueSet = true;
			arg->xTemp_Struct.bThresholdSet = false;

		} else if (strTemp[0] ==  'c'){
			arg->xTemp_Struct.bValueSet = false;
			arg->xTemp_Struct.bThresholdSet = false;
		}

		if (strcmp(strHum, " ") && ((char)strHum[0] != '\0') && (char) strHum[0] != 'c')
		{
			/* Set hum */
			setValue("do=setHum,", strHum);
			strtoIntValue(strHum, &arg->xHum_Struct.uValue);
			arg->xHum_Struct.bValueSet = true;
			arg->xHum_Struct.bThresholdSet = false;

		} else if (strHum[0] ==  'c'){
			arg->xHum_Struct.bValueSet = false;
			arg->xHum_Struct.bThresholdSet = false;
		}

		Server_Handle("/setForm", Link_ID, arg);
	}


	/* Ranges */
	else if (Look_for("/setRange", buftostoreheader) == 1)
	{
		controlData paramTmp = {0};

		GetDataFromBuffer("minTemp=", "&", buftostoreheader, paramTmp.pcMinTemp);
		GetDataFromBuffer("maxTemp=", "&", buftostoreheader, paramTmp.pcMaxTemp);
		GetDataFromBuffer("minHum=", "&", buftostoreheader, paramTmp.pcMinHum);
		GetDataFromBuffer("maxHum=", " HTTP", buftostoreheader, paramTmp.pcMaxHum);
		Handle_controlData(&paramTmp, arg);

		Server_Handle("/setRange", Link_ID, arg);
	}


	else if (Look_for("/main", buftostoreheader) == 1)
	{

		Server_Handle("/main", Link_ID, arg);
	}


	/* Turn on and off commands */
	else if (Look_for("/turnon", buftostoreheader) == 1)
	{

		Uart_sendstring("do=turnOn.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}

	else if (Look_for("/turnoff", buftostoreheader) == 1)
	{

		Uart_sendstring("do=turnOff.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}


	/* Schedule commands */
	else if (Look_for("/timer", buftostoreheader) == 1)
	{
		xScheduledTaskParams_t xScheduleParams = {0};

		GetDataFromBuffer("com=", "&", buftostoreheader, xScheduleParams.command);
		GetDataFromBuffer("days=", "&", buftostoreheader, xScheduleParams.days);
		GetDataFromBuffer("hs=", "&", buftostoreheader, xScheduleParams.hours);
		GetDataFromBuffer("min=", "&", buftostoreheader, xScheduleParams.minutes);
		GetDataFromBuffer("sec=", "&", buftostoreheader, xScheduleParams.seconds);
		GetDataFromBuffer("arg1=", "&", buftostoreheader, xScheduleParams.param1);
		GetDataFromBuffer("arg2=", " HTTP", buftostoreheader, xScheduleParams.param2);

		HandleScheduleData(&xScheduleParams, xSharedArgs);

		Server_Handle("/timer", Link_ID, arg);
	}


	/* Special buttons */
	else if (Look_for("/special", buftostoreheader) == 1)
	{
		char command[20] = {0};
		char arg1[20] = {0};
		char arg2[20] = {0};
		char buf[64] = {0};

		GetDataFromBuffer("com=", "&", buftostoreheader, command);
		GetDataFromBuffer("arg1=", "&", buftostoreheader, arg1);
		GetDataFromBuffer("arg2=", " HTTP", buftostoreheader, arg2);

		// send special1 uart com

		sprintf(buf, "do=%s,%s,%s.\r\n", command, arg1, arg2);
		Uart_sendstring(buf, device_uart);
		Server_Handle("/special", Link_ID, arg);
	}

	else if (Look_for("/favicon.ico", buftostoreheader) == 1)
		(void)0;

	else
	{
		Server_Handle("/main", Link_ID, arg);
	}
//timeout:
//	(void)0;
}


static void bufclr (char *buf)
{
	int len = strlen (buf);
	for (int i=0; i<len; i++) buf[i] = '\0';
}


int wait_timeout(char *str, UART_HandleTypeDef *uart, uint32_t times){
	int tmp = -10;
	while (1) {
		tmp = Wait_for_timeout(str, uart, times);
		if (tmp == WAIT_OK) {
			return 1;
		} else if (tmp == WAIT_TIMEOUT) {
			return 0;
		}
	}
}


void ESP_Send_Multi (char *APIkey, int numberoffileds, uint8_t value[])
{
	char local_buf[500] = {0};
	char local_buf2[30] = {0};
	char field_buf[200] = {0};

	Uart_sendstring("AT+CIPSTART=2,\"TCP\",\"184.106.153.149\",80\r\n", wifi_uart);
	if (!(wait_timeout("OK\r\n", wifi_uart, 10000000))) {
		goto reset;
	}


	sprintf (local_buf, "GET /update?api_key=%s", APIkey);
	for (int i=0; i<numberoffileds; i++)
	{
		sprintf(field_buf, "&field%d=%u", i+1, value[i]);
		strcat (local_buf, field_buf);
	}

	strcat(local_buf, "\r\n");
	int len = strlen (local_buf);

	/* Set a small delay to allow cipstart command to be processed */
	HAL_Delay(750);

	sprintf (local_buf2, "AT+CIPSEND=2,%d\r\n", len);
	Uart_sendstring(local_buf2, wifi_uart);

	if (!(wait_timeout(">", wifi_uart, 10000000))) {
		goto reset;
	}


	Uart_sendstring (local_buf, wifi_uart);

	if (!(wait_timeout("SEND OK\r\n", wifi_uart, 10000000))) {
		goto reset;
	}

	if (!(wait_timeout("CLOSED", wifi_uart, 10000000))) {
		goto reset;
	}

reset:
	bufclr(local_buf);
	bufclr(local_buf2);
	bufclr(field_buf);

	Ringbuf_init();
	Uart_flush(wifi_uart);

}
