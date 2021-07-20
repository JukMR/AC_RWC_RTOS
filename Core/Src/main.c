/* USER CODE BEGIN Header */
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdio.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"

#include "ESP_DATA_HANDLER.h"
#include "ESPDataLogger.h"
#include "DHT.h"

/* Remove after re-implementation of uart_Sendstring */
#include "UartRingbuffer.h"

#include "led.h"

#define wifi_uart &huart1
#define uart_command &huart2

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
void *ledRed;
void *ledBlue;
void *ledOrange;
void *ledGreen;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
//void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void vSendDataThingSpeakTask(void *pvParameters)
{
  uint8_t buffer[2] = {0,0};
  DHT_DataTypedef *tmp;

  TickType_t xLastWakeTime = xTaskGetTickCount();


  for (;;)
  {

    tmp = (DHT_DataTypedef *)pvParameters;

    buffer[0] = tmp->Temperature;
    buffer[1] = tmp->Humidity;

    buffer[0] = 30;
    buffer[1] = 40;

    ESP_Send_Multi("U6123BFR6YNW5I4V", 2, buffer);

    // wait 15s between sends
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(15000));
  }
}

void vRefreshWebserverTask(void * pvParameters)
{
//  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
	  Server_Start();

//	  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(4000));
  }
}

void vSetTemp(int Value)
{
  char buffer[40] = {0};
  sprintf(buffer, "do=setTemp,%d.\r\n", Value);

  // Implement this
  Uart_sendstring(buffer, uart_command);
}

void vSetHum(int Value)
{
  char buffer[40] = {0};
  sprintf(buffer, "do=setHum,%d.\r\n", Value);

  // Implement this
  Uart_sendstring(buffer, uart_command);
}

void vControlTempHum(void *ptr)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {

    ControlTempParams *param1 = (ControlTempParams *)ptr;

    // Temperature Control

    // Threshold activated
    if (param1->temp_Struct.thresholdSet)
    {

      // temperature is lower than threshold
      if (param1->dhtPolledData.Temperature < param1->temp_Struct.min)
      {
        vSetTemp(param1->temp_Struct.max);

        // temperature is higher than threshold
      }
      else if (param1->dhtPolledData.Temperature > param1->temp_Struct.max)
      {
        vSetTemp(param1->temp_Struct.min);
      }

      // temperature value set
    }
    else if (param1->temp_Struct.valueSet)
    {
      if (param1->dhtPolledData.Temperature != param1->temp_Struct.value)
      {
        vSetTemp(param1->temp_Struct.value);
      }
    }

    // Humidity Control

    // Threshold activated
    if (param1->hum_Struct.thresholdSet)
    {

      // temperature is lower than threshold
      if (param1->dhtPolledData.Humidity < param1->hum_Struct.min)
      {
        vSetHum(param1->hum_Struct.max);

        // temperature is higher than threshold
      }
      else if (param1->dhtPolledData.Humidity > param1->hum_Struct.max)
      {
        vSetHum(param1->hum_Struct.min);
      }

      // temperature value set
    }
    else if (param1->hum_Struct.valueSet)
    {
      if (param1->dhtPolledData.Humidity != param1->hum_Struct.value)
      {
        vSetHum(param1->hum_Struct.value);
      }
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));
  }
}

void vTurnOn()
{
  char buffer[20] = {0};
  sprintf(buffer, "do=turnon.\r\n");
  Uart_sendstring(buffer, uart_command);
}

void vTurnOff()
{
  char buffer[20] = {0};
  sprintf(buffer, "do=turnoff.\r\n");
  Uart_sendstring(buffer, uart_command);
}

void vTaskGetDataDHT(void *ptr)
{

  char buf[30];
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {

    DHT_DataTypedef *data_struct = (DHT_DataTypedef *)ptr;
    DHT_GetData(data_struct);

    sprintf(buf, "Temp: %d, Hum:%d\r\n", (int)data_struct->Temperature, (int)data_struct->Humidity);

    /* Improve this somehow */
    Uart_sendstring(buf, uart_command);

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(6000));
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

//  MX_USART6_UART_Init();

  /* USER CODE BEGIN 2 */

//  LED_Init();

//  LED_on(ledOrange);

  /* Initialize Uart library */
  //  Ringbuf_init();

  ESP_Init("Diagon Alley 2.4GHz", "hayunboggartenlaalacena", "192.168.0.200");

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  //  MX_FREERTOS_Init();
  /* Start scheduler */
  //  osKernelStart();

  ControlTempParams param1;

  param1.dhtPolledData.Humidity = 10;
  param1.dhtPolledData.Temperature = 10;

  param1.temp_Struct.min = 5;
  param1.temp_Struct.max =20;
  param1.temp_Struct.thresholdSet = true;
  param1.temp_Struct.valueSet = false;


  param1.hum_Struct.min = 80;
  param1.hum_Struct.max = 95;
  param1.temp_Struct.thresholdSet = true;
  param1.hum_Struct.valueSet = false;

  while(1) {
	  vRefreshWebserverTask( NULL);
  }

//  xTaskCreate(vTaskGetDataDHT, "vTaskGetData", 1000, &param1.dhtPolledData, 3, NULL);
//  xTaskCreate(vControlTempHum, "controlTemp", 1000, &param1, 2, NULL);
//  xTaskCreate(vSendDataThingSpeakTask, "SendDataThingSpeak", 1000, &param1.dhtPolledData, 1, NULL);
//    xTaskCreate( vRefreshWebserverTask, "RefreshWebserver", 1200, NULL, 1, NULL);
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

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
