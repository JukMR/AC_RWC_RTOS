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

char buffer[20];

controlData param;

char *home_start = "<!DOCTYPE html>\n\
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
\n\
        <div>\n\
\n\
            <h2> Encendido/Apagado</h2>\n\
            <button onclick=location.href=\"turnon\"> Encender AC </button>\n\
            &nbsp;&nbsp;\n\
            <button onclick=location.href=\"turnoff\"> Apagar AC </button>\n\
\n\
        </div>\n\
\n\
        <div>\n\
            <h2> Elegir temperatura </h2>\n\
            <form action=\"store.html\">\n\
                <label for=\"temp\"> Temperatura: </label>\n\
                <input type=\"text\" id=\"temp\" name=\"temp\" value=\"\"> <br> </br>\n\
            </form>\n\
        </div>\n\
\n\
        <div>\n\
            <h2> Elegir humedad </h2>\n\
            <form action=\"store.html\">\n\
                <label for=\"hum\"> Humedad: </label>\n\
                <input type=\"text\" id=\"hum\" name=\"hum\" value=\"\"> <br> </br>\n\
            </form>\n\
        </div>\n\
\n\
\n\
        <h2>Elegir rangos</h2>\n\
        <form action=\"/page1\">\n\
\n\
            <p> Temperature Range </p>\n\
            <i>(Value between -50 and 100)</i><br></br>\n\
\n\
            <label for=\"minTemp\">Minimum:</label>\n\
            <input type=\"text\" id=\"minTemp\" name=\"minTemp\" value=\"\"><br></br>\n\
\n\
            <label for=\"maxTemp\">Maximum:</label>\n\
            <input type=\"text\" id=\"maxTemp\" name=\"maxTemp\" value=\"\"><br><br>\n\
\n\
\n\
            <p> Humidity Range </p>\n\
            <i>(Value between 0 and 100)</i><br></br>\n\
\n\
            <label for=\"minHum\">Minimum:</label>\n\
            <input type=\"text\" id=\"minHum\" name=\"minHum\" value=\"\"><br></br>\n\
\n\
            <label for=\"maxHum\">Maximum:</label>\n\
            <input type=\"text\" id=\"maxHum\" name=\"maxHum\" value=\"\"><br><br>\n\
            <input type=\"submit\" value=\"Submit\">\n\
\n\
        </form><br><br>\n";

char * home_end ="  <button onclick=location.href=\"https://thingspeak.com/channels/1439978\"> Watch thingSpeak data</button> <br>\n\
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

	char *page1 = "<!DOCTYPE html>\n\
<html>\n\
\n\
<body>\n\
    <div style=\"text-align: center;\">\n\
        <h1 style=\"text-align: center;\"> Servidor Esp8266 con DHT11</h1>\n\
\n\
        <h2> Command send successfully </h2>\n\
        <p> Click below to return the main page</p>\n\
        <form action=\"main.html\" style=\"text-align: center;\">\n\
            <input type=\"submit\" value=\"Go back\">\n\
        </form><br><br>\n\
\n\
    </div>\n\
\n\
</body>\n\
\n\
</html>";




/*****************************************************************************************************************************************/

void ESP_Init (char *SSID, char *PASSWD, char *STAIP)
{
	char data[80];

	Ringbuf_init();

	Uart_sendstring("AT+RST\r\n", wifi_uart);
	HAL_Delay(2000);

	/********* AT **********/
	Uart_flush(wifi_uart);
	Uart_sendstring("AT\r\n", wifi_uart);
	while(!(Wait_for("OK\r\n", wifi_uart)));


	/********* AT+CWMODE=1 **********/
	Uart_flush(wifi_uart);
	Uart_sendstring("AT+CWMODE=1\r\n", wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/* Set Static IP Address */
	/********* AT+CWSTAIP=IPADDRESS **********/
	Uart_flush(wifi_uart);
	sprintf (data, "AT+CIPSTA=\"%s\"\r\n", STAIP);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));

	/********* AT+CWJAP="SSID","PASSWD" **********/
	Uart_flush(wifi_uart);
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
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




int Server_Send (char *str, int Link_ID)
{
	int len = strlen (str);
	char data[80];
	Uart_flush(wifi_uart);
	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, len);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for(">", wifi_uart)));
	Uart_sendstring (str, wifi_uart);
	while (!(Wait_for("SEND OK", wifi_uart)));
	Uart_flush(wifi_uart);
	sprintf (data, "AT+CIPCLOSE=%d\r\n",Link_ID);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));
	return 1;
}

int Server_Send_main (char *start, char *end, int Link_ID)
{
	int lenStart = strlen (start);
	int lenEnd = strlen(end);
	char data[80];
	Uart_flush(wifi_uart);
	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenStart);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for(">", wifi_uart)));
	Uart_sendstring (start, wifi_uart);
	while (!(Wait_for("SEND OK", wifi_uart)));
	Uart_flush(wifi_uart);

	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, lenEnd);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for(">", wifi_uart)));
	Uart_sendstring (end, wifi_uart);
	while (!(Wait_for("SEND OK", wifi_uart)));
	Uart_flush(wifi_uart);

	sprintf (data, "AT+CIPCLOSE=%d\r\n", Link_ID);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));
	return 1;
}

void Server_Handle (char *str, int Link_ID)
{
	char datatosend[4096] = {0};
	if (!(strcmp (str, "/page1")))
	{
		sprintf(datatosend, page1);
		Server_Send(datatosend, Link_ID);
	}

//	else if (!(strcmp (str, "/page2")))
//	{
//		char localbuf[2048];
//		sprintf(datatosend, page2_Top);
//		strcat (datatosend, table);
//		int bufsize = (sizeofuser (user));
//		for (int i=0; i<bufsize; i++)
//		{
//			sprintf (localbuf, "<tr><td>%s %s</td>	<td>%s</td></tr>",user[i].firstname,user[i].lastname,user[i].age);
//			strcat (datatosend, localbuf);
//		}
//		strcat (datatosend, "</table>");
//		strcat(datatosend, page2_end);
//		Server_Send(datatosend, Link_ID);
//	}
	else
	{

//		sprintf (datatosend, buffer);
		Server_Send_main(home_start, home_end,  Link_ID);

	}

}


static void strtoIntValue (char *str, int *value) {

    if ((strcmp(str, ""))) *value = atoi(str);

}

static void Handle_controlData(controlData *tmp, controlData *param) {
    int minTemp, maxTemp, minHum, maxHum;
    minTemp = maxTemp = minHum = maxHum = 999;

    strtoIntValue(tmp->minTemp, &minTemp);
    strtoIntValue(tmp->maxTemp, &maxTemp);
    strtoIntValue(tmp->minHum, &minHum);
    strtoIntValue(tmp->maxHum, &maxHum);


    if ((minTemp >= -50) && (maxTemp <= 100) && (minTemp <= maxTemp)) {
        // call change threshold function
        sprintf(param->minTemp, "%d", minTemp);
        sprintf(param->maxTemp, "%d", maxTemp);
    }

    if ((minHum >= 0) && (maxHum <= 100) && (minHum <= maxHum)) {
        // call change threshold function
        sprintf(param->minHum, "%d", minHum);
        sprintf(param->maxHum, "%d", maxHum);
    }

}

void Server_Start (void)
{
	extern void *ledOrange;
	extern void LED_on(void *led);
	extern void LED_off(void *led);

	char buftostoreheader[128] = {0};
	char Link_ID;
	while (!(Get_after("+IPD,", 1, &Link_ID, wifi_uart)));

	Link_ID -= 48;
	while (!(Copy_upto(" HTTP/1.1", buftostoreheader, wifi_uart)));

	controlData paramTmp = {0};


	if (Look_for("/page1", buftostoreheader) == 1)
	{
		GetDataFromBuffer("minTemp=", "&", buftostoreheader, paramTmp.minTemp);
		GetDataFromBuffer("maxTemp=", "&", buftostoreheader, paramTmp.maxTemp);
		GetDataFromBuffer("minHum=", "&", buftostoreheader, paramTmp.minHum);
		GetDataFromBuffer("maxHum=", " HTTP", buftostoreheader, paramTmp.maxHum);
		Handle_controlData(&paramTmp, &param);
		Server_Handle("/page1",Link_ID);
	}

	else if (Look_for("/home", buftostoreheader) == 1)
	{
		Server_Handle("/home",Link_ID);
	}

	else if (Look_for("/turnon", buftostoreheader) == 1)
	{
		LED_on(ledOrange);

		Server_Handle("/page1",Link_ID);
	}

	else if (Look_for("/turnoff", buftostoreheader) == 1)
	{
		LED_off(ledOrange);
		Server_Handle("/page1",Link_ID);
	}

	else if (Look_for("/special1", buftostoreheader) == 1)
	{
		// send special1 uart com
		Server_Handle("/page1",Link_ID);;
	}

	else if (Look_for("/special2", buftostoreheader) == 1)
	{
		// send special2 uart com
		Server_Handle("/page1",Link_ID);
	}

	else if (Look_for("/special3", buftostoreheader) == 1)
	{
		// send special3 uart com
		Server_Handle("/page1",Link_ID);
	}

	else if (Look_for("/favicon.ico", buftostoreheader) == 1);

	else
	{
		Server_Handle("/ ", Link_ID);
	}
}








