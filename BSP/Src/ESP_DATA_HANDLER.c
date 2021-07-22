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

#include "ESP_DATA_HANDLER.h"
#include "UartRingbuffer.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define wifi_uart &huart1
#define device_uart &huart2

#define WAIT_TIMEOUT 3
#define WAIT_OK 1
#define WAIT_ERROR 2

char buffer[64];

char *home_header =
	"<!DOCTYPE html>\n\
<html lang=\'en\'>\n\
\n\
<head>\n\
    <meta charset=\"UTF 8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <title Servidor Esp8266></title>\n\
</head>\n\
\n\
<body>\n\
    <div style=\"text-align: center;\">\n\
\n\
        <h1> Servidor Esp8266 con DHT11</h1>\n\
        <div>\n\
\n\
            <h2> Encendido/Apagado</h2>\n\
            <button onclick=location.href=\"turnon\"> Encender AC </button>\n\
            &nbsp;&nbsp;\n\
            <button onclick=location.href=\"turnoff\"> Apagar AC </button>\n\
\n\
        </div>";

char *home_values =

	"<div>\n\
            <h2>Temperatura actual: 20</h2>\n\
            <h2>Humedad actual es: 20</h2>\n\
        </div>";

char *home_tail =

	"<form action=\"/setForm\">\n\
            <h2> Elegir temperatura </h2>\n\
            <label for=\"temp\"> Temperatura: </label>\n\
            <input type=\"text\" id=\"idSetTemp\" name=\"setTemp\" value=\"\"> <br> </br>\n\
\n\
            <h2> Elegir humedad </h2>\n\
            <label for=\"hum\"> Humedad: </label>\n\
            <input type=\"text\" id=\"idSetHum\" name=\"setHum\" value=\"\"> <br> </br>\n\
            <input type=\"submit\" value=\"Set Values\">\n\
\n\
        </form>\n\
\n\
        <h2>Elegir rangos</h2>\n\
        <form action=\"/setRange\">\n\
\n\
            <p> Temperature Range </p>\n\
            <i>(Value between 0 and 50)</i><br></br>\n\
\n\
            <label for=\"minTemp\">Minimum:</label>\n\
            <input type=\"text\" id=\"minTemp\" name=\"minTemp\" value=\"\"><br></br>\n\
\n\
            <label for=\"maxTemp\">Maximum:</label>\n\
            <input type=\"text\" id=\"maxTemp\" name=\"maxTemp\" value=\"\"><br><br>\n\
\n\
\n\
            <p> Humidity Range </p>\n\
            <i>(Value between 5 and 95)</i><br></br>\n\
\n\
            <label for=\"minHum\">Minimum:</label>\n\
            <input type=\"text\" id=\"minHum\" name=\"minHum\" value=\"\"><br></br>\n\
\n\
            <label for=\"maxHum\">Maximum:</label>\n\
            <input type=\"text\" id=\"maxHum\" name=\"maxHum\" value=\"\"><br><br>\n\
            <input type=\"submit\" value=\"Set Ranges\">\n\
\n\
        </form><br><br>\n\
\n\
        <button onclick=location.href=\"https://thingspeak.com/channels/1439978\"> Watch thingSpeak data</button> <br>\n\
        <br>\n\
\n\
        <div>\n\
            <h2> Special buttons for general use </h2>\n\
            <button onclick=location.href=\"special1\"> Special 1 </button>\n\
            <button onclick=location.href=\"special2\"> Special 2 </button>\n\
            <button onclick=location.href=\"special3\"> Special 3 </button>\n\
        </div>\n\
    </div>\n\
</body>\n\
</html>";

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

/*****************************************************************************************************************************************/

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


int Server_Send_main(char *start, char *end, int Link_ID, DHT_DataTypedef *dht_struct)
{
	int lenStart = strlen(start);
	char data[80];
	char body[200];

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

	sprintf(body, "<div><h2>Temperatura actual: %u</h2><h2>Humedad actual es: %u</h2></div>", dht_struct->Temperature, dht_struct->Humidity);
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

	Uart_flush(wifi_uart);

	sprintf(data, "AT+CIPCLOSE=%d\r\n", Link_ID);
	Uart_sendstring(data, wifi_uart);


//	while (!(Wait_for("OK\r\n", wifi_uart)));
	if (!(wait_timeout("OK\r\n", wifi_uart, 10000000))) goto reset;

reset:
	return 1;
}


void Server_Handle(char *str, int Link_ID, ControlTempParams *arg)
{
	char datatosend[4096] = {0};

	if (!(strcmp(str, "/receiveCommand")))
	{
		sprintf(datatosend, receiveCommand);
		Server_Send(datatosend, Link_ID);
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

	else if (!(strcmp(str, "/main")))
	{
		Server_Send_main(home_header, home_tail, Link_ID, &arg->dhtPolledData);
	}

	else
	{

		/* I think we should not reach here */
		sprintf(datatosend, error);
		Server_Send(datatosend, Link_ID);
	}
}


static void strtoIntValue(char *str, int *value)
{

	if ((strcmp(str, "")))
		*value = atoi(str);
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


static void Handle_controlData(controlData *tmp, ControlTempParams *arg)
{
	int minTemp, maxTemp, minHum, maxHum;
	minTemp = maxTemp = minHum = maxHum = 999;

	strtoIntValue(tmp->minTemp, &minTemp);
	strtoIntValue(tmp->maxTemp, &maxTemp);
	strtoIntValue(tmp->minHum, &minHum);
	strtoIntValue(tmp->maxHum, &maxHum);

	if ((minTemp >= 0) && (maxTemp <= 50) && (minTemp <= maxTemp))
	{
		// call change threshold function
		arg->temp_Struct.min = minTemp;
		arg->temp_Struct.max = maxTemp;

		/* Send temp range change command */
		setRanges("do=temprange,", minTemp, maxTemp);
	}

	if ((minHum >= 5) && (maxHum <= 95) && (minHum <= maxHum))
	{
		// call change threshold function
		arg->hum_Struct.min = minHum;
		arg->hum_Struct.max = maxHum;

		/* Send hum range change command */
		setRanges("do=humrange,", minHum, maxHum);
	}
}


void Server_Start(ControlTempParams *arg)
{
	char buftostoreheader[128] = {0};
	char Link_ID;
	while (!(Get_after("+IPD,", 1, &Link_ID, wifi_uart)));

	Link_ID -= 48;
	while (!(Copy_upto(" HTTP/1.1", buftostoreheader, wifi_uart)));

	controlData paramTmp = {0};
	char strTemp[5] = {0};
	char strHum[5] = {0};

	if (Look_for("/setForm", buftostoreheader) == 1)
	{
		GetDataFromBuffer("setTemp=", "&", buftostoreheader, strTemp);
		GetDataFromBuffer("setHum=", "HTTP", buftostoreheader, strHum);

		if (strcmp(strTemp, " ") && ((char)strTemp[0] != '\0'))
		{
			/* Set temp */
			setValue("do=settemp,", strTemp);
			strtoIntValue(strTemp, &arg->temp_Struct.value);
			arg->temp_Struct.valueSet = true;
		}

		if (strcmp(strHum, " "))
		{
			/* Set hum */
			setValue("do=sethum,", strHum);
			strtoIntValue(strHum, &arg->hum_Struct.value);
			arg->hum_Struct.valueSet = true;
		}

		Server_Handle("/setForm", Link_ID, arg);
	}

	else if (Look_for("/setRange", buftostoreheader) == 1)
	{
		GetDataFromBuffer("minTemp=", "&", buftostoreheader, paramTmp.minTemp);
		GetDataFromBuffer("maxTemp=", "&", buftostoreheader, paramTmp.maxTemp);
		GetDataFromBuffer("minHum=", "&", buftostoreheader, paramTmp.minHum);
		GetDataFromBuffer("maxHum=", " HTTP", buftostoreheader, paramTmp.maxHum);
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

		Uart_sendstring("do=turnon.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}

	else if (Look_for("/turnoff", buftostoreheader) == 1)
	{

		Uart_sendstring("do=turnoff.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}

	/* Special buttons */
	else if (Look_for("/special1", buftostoreheader) == 1)
	{
		// send special1 uart com
		Uart_sendstring("do=special1.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}

	else if (Look_for("/special2", buftostoreheader) == 1)
	{
		// send special2 uart com
		Uart_sendstring("do=special2.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}

	else if (Look_for("/special3", buftostoreheader) == 1)
	{
		// send special3 uart com
		Uart_sendstring("do=special3.\r\n", device_uart);
		Server_Handle("/receiveCommand", Link_ID, arg);
	}

	else if (Look_for("/favicon.ico", buftostoreheader) == 1);

	else
	{
		Server_Handle("/main", Link_ID, arg);
	}
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

	HAL_Delay(750);

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
