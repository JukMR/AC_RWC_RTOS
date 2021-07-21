/*
 * ESPDataLogger.c
 *
 *  Created on: May 26, 2020
 *      Author: controllerstech
 */

#include "UartRingbuffer.h"
#include "ESPDataLogger.h"
#include "stdio.h"
#include "string.h"

extern UART_HandleTypeDef huart1;

#define wifi_uart &huart1


#define WAIT_TIMEOUT 3
#define WAIT_OK 1
#define WAIT_ERROR 2


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

